#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "central-office.h"
#include "conf.h"
#include "cons.h"
#include "tweet.h"
#include "util.h"
#include "youtube-access.h"

using namespace std;

namespace YoutubeAccess {
  vector<Tweet*> _entries;

  struct Access {
    Tweet* t;
    double dist;

    Access(Tweet* t_, double dist_)
      : t(t_), dist(dist_)
    {}
  };

  std::ostream& operator<< (std::ostream& os, const Access& a) {
    os << boost::format("%s %d") % (*(a.t)) % a.dist;
    return os;
  }

  struct cmpCO {
    bool operator()(const CentralOffice* a, const CentralOffice* b) {
      return (a->id < b->id);
    }
  };

  map<const CentralOffice*, vector<Access*>, cmpCO> _co_accesses;
  mutex _co_accesses_mutex;


  const vector<Tweet*>& Entries() {
    return _entries;
  }

  void _LoadOps() {
    string fn = Conf::GetFn("youtube_accesses");

    // Unzip if needed
    if (! boost::filesystem::exists(fn)) {
      string fn_7z = fn + ".7z";
      if (boost::filesystem::exists(fn_7z)) {
        string dn = boost::filesystem::path(fn).parent_path().string();
        string fn1 = boost::filesystem::path(fn).filename().string();
        string cmd = str(boost::format("cd %s && 7z e %s.7z") % dn % fn1);
        Util::RunSubprocess(cmd);
      } else
        THROW("Unexpected");
    }

    Cons::MT _(boost::format("Loading YouTube workload from file %s ...") % fn);

    ifstream ifs(fn.c_str(), ios::binary);
    if (! ifs.is_open())
      THROW(boost::format("unable to open file %1%") % fn);

    size_t e_size;
    ifs.read((char*)&e_size, sizeof(e_size));

    for (size_t i = 0; i < e_size; i ++) {
      Tweet* tw = new Tweet(ifs);
      _entries.push_back(tw);

      if (i % 10000 == 0) {
        Cons::ClearLine();
        Cons::Pnnl(boost::format("%.2f%%") % (100.0 * i / e_size));
      }
    }
    Cons::ClearLine();
    Cons::P("100.00%");

    Cons::P(boost::format("loaded %d ops") % _entries.size());
  }

  void _GenFileForCoordPlot() {
    // Make a file with coordinate, in_usa field for plotting
    Cons::MT _("Generating a (coordinate, in_usa) file ...");

    string fn = str(boost::format("%s/coord-in_usa-%s") % Conf::DnOut()
        % boost::filesystem::path(Conf::GetStr("youtube_workload")).filename().string());
    ofstream ofs(fn);
    for (auto e: _entries) {
      ofs << boost::format("%f %f\n")
        % e->geo_lati
        % e->geo_longi;
    }
    ofs.close();
    Cons::P(boost::format("created file %s %d") % fn % boost::filesystem::file_size(fn));
    exit(0);
  }

  void Load() {
    _LoadOps();

    //_GenFileForCoordPlot();
  }

  void FreeMem() {
    if (_entries.size() == 0)
      return;

    for (auto e: _entries)
      delete e;
    _entries.clear();
  }

  //void _MapSerial() {
  //  // Serial processing is not that slow. Takes about 1 min.
  //  int i = 0;
  //  size_t e_size = _entries.size();
  //  for (const auto t: _entries) {
  //    CentralOffice* co = CentralOffices::GetNearest(t->geo_lati, t->geo_longi);
  //    if (co == nullptr)
  //      THROW("Unexpected");
  //    //Cons::P(boost::format("%s") % *co);

  //    auto it = _co_accesses.find(co);
  //    if (it == _co_accesses.end()) {
  //      vector<const Tweet*> tweets;
  //      tweets.push_back(t);
  //      _co_accesses.emplace(co, tweets);
  //    } else {
  //      it->second.push_back(t);
  //    }

  //    if (i % 10000 == 0) {
  //      Cons::ClearLine();
  //      Cons::Pnnl(boost::format("%.2f%%") % (100.0 * i / e_size));
  //    }
  //    i ++;
  //  }
  //  Cons::ClearLine();
  //  Cons::P("100.00%");

  //  Cons::P(boost::format("mapped %d YouTube accesses to COs") % i);
  //}

  void _MapP0(unsigned int i_begin, unsigned int i_end) {
    try {
      map<const CentralOffice*, vector<Access*> > co_accesses;

      for (unsigned int i = i_begin; i < i_end; i ++) {
        Tweet* t = _entries[i];
        CentralOffice* co = CentralOffices::GetNearest(t->geo_lati, t->geo_longi);
        if (co == nullptr)
          THROW("Unexpected");
        //Cons::P(boost::format("%s") % *co);

        // Calc the dist between t and co.
        //   Let's get them all for a start. You might want just an average value or a CDF, but let's not optimize until needed.
        double dist = Util::ArcInMeters(t->geo_lati, t->geo_longi, co->c.lat, co->c.lon);
        Access* acc = new Access(t, dist);

        auto it = co_accesses.find(co);
        if (it == co_accesses.end()) {
          vector<Access*> accesses;
          accesses.push_back(acc);
          co_accesses.emplace(co, accesses);
        } else {
          it->second.push_back(acc);
        }
      }

      {
        // Merge the local co_accesses to the global one
        lock_guard<mutex> lock(_co_accesses_mutex);

        for (auto i: co_accesses) {
          const CentralOffice* co = i.first;
          vector<Access*>& accesses = i.second;

          auto it = _co_accesses.find(co);
          if (it == _co_accesses.end()) {
            _co_accesses.emplace(co, accesses);
          } else {
            copy(accesses.begin(), accesses.end(), back_inserter(it->second));
          }
        }
      }
    } catch (const exception& e) {
      cerr << "Got an exception: " << e.what() << "\n";
      exit(1);
    }
  }

  void _MapParallel() {
    {
      Cons::MT _("Mapping in parallel ...");
      unsigned int conc = thread::hardware_concurrency();
      Cons::P(boost::format("%d hardware threads") % conc);

      size_t e_size = _entries.size();
      int items_per_thread = ceil(double(e_size) / conc);

      vector<thread> threads;
      for (unsigned int i = 0; i < conc; i ++) {
        // [i_begin, i_end)
        unsigned int i_begin = i * items_per_thread;
        unsigned int i_end = (i + 1) * items_per_thread;
        if (e_size < i_end)
          i_end = e_size;
        if (i_end < i_begin)
          i_begin = i_end;
        //Cons::P(boost::format("%2d %d %d") % i % i_begin % i_end);
        threads.push_back(thread(_MapP0, i_begin, i_end));
      }
      for (auto& t: threads)
        t.join();
    }

    {
      Cons::MT _("Sorting vector<Access*> by their timestamps ...");
      for (auto& i: _co_accesses) {
        vector<Access*>& accesses = i.second;
        //for (auto t: accesses) {
        //  Cons::P(boost::format("%s") % *t);
        //}

        sort(accesses.begin(), accesses.end(),
            //[](const Access* & a, const Access* & b) -> bool
            [](const Access* a, const Access* b) -> bool
            {
              return a->t->created_at < b->t->created_at;
            });

        // Check
        //for (auto t: i.second) {
        //  Cons::P(boost::format("AA %s") % *t);
        //}
      }
    }

    // Compare the result with the serial one. Well. I think the result is correct.
  }

  void MapAccessToCentraloffice() {
    {
      Cons::MT _("Mapping YouTube acceses to central offices ...");
      //_MapSerial();
      _MapParallel();
    }
    {
      Cons::MT _("Generating output file ...");
      string fn = str(boost::format("%s/centraloffice-videoaccesses") % Conf::DnOut());
      ofstream ofs(fn);
      ofs << "# central_office_id latitude longitude num_video_accesses\n";
      ofs << "#   tweet_id user_id created_at latitude longitude youtube_video_id dist_to_CO\n";
      ofs << "\n";
      for (auto i: _co_accesses) {
        // Skip one-request COs. Not helpful for comparing the cache placement algorithms.
        //   You could skip less-than-n-request COs in general. Not sure if I will need it though.
        if (i.second.size() <= 1)
          continue;

        ofs << boost::format("%s %d\n") % *i.first % i.second.size();

        for (auto t: i.second)
          ofs << boost::format("  %s\n") % *t;
      }
      ofs.close();
      Cons::P(boost::format("created %s %d") % fn % boost::filesystem::file_size(fn));
    }
  }
}
