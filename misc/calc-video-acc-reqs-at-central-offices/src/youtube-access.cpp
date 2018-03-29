#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "conf.h"
#include "cons.h"
#include "op-youtube.h"
#include "util.h"
#include "youtube-access.h"

using namespace std;

namespace YoutubeAccess {
  // youtube
  vector<Op*> _entries;
  size_t _num_writes = 0;
  size_t _num_reads = 0;

  const vector<Op*>& Entries() {
    return _entries;
  }

  size_t NumReads() {
    return _num_reads;
  }

  size_t NumWrites() {
    return _num_writes;
  }

  void _LoadOps() {
    string fn = Conf::GetFn("youtube_workload");
    string dn = boost::filesystem::path(fn).parent_path().string();
    string fn1 = boost::filesystem::path(fn).filename().string();

    // Unzip if a zipped file exist
    if (! boost::filesystem::exists(fn)) {
      string fn_7z = fn + ".7z";
      if (boost::filesystem::exists(fn_7z)) {
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

    long num_in_usa = 0;
    long num_outside_usa = 0;
    for (size_t i = 0; i < e_size; i ++) {
      Op* e = new OpYoutube(ifs);
      _entries.push_back(e);

      if (e->type == Op::Type::W)
        _num_writes ++;
      else if (e->type == Op::Type::R)
        _num_reads ++;

      if (e->in_usa == 'Y')
        num_in_usa ++;
      else
        num_outside_usa ++;

      if (i % 10000 == 0) {
        Cons::ClearLine();
        Cons::Pnnl(boost::format("%.2f%%") % (100.0 * i / e_size));
      }
    }
    Cons::ClearLine();
    Cons::P("100.00%");

    Cons::P(boost::format("loaded %d ops. %d Ws, %d Rs. in_usa=%d. outside_usa=%d")
        % _entries.size() % _num_writes % _num_reads % num_in_usa % num_outside_usa);
  }

  void _TestInUsa() {
    // Make a file with coordinate, in_usa field for plotting
    Cons::MT _("Generating a (coordinate, in_usa) file ...");
    string dn = str(boost::format("%s/../.output") % boost::filesystem::path(__FILE__).parent_path().string());
    boost::filesystem::create_directories(dn);

    string fn = str(boost::format("%s/coord-in_usa-%s") % dn
        % boost::filesystem::path(Conf::GetStr("youtube_workload")).filename().string());
    ofstream ofs(fn);
    for (auto e: _entries) {
      ofs << boost::format("%f %f %c\n")
        % e->lat
        % e->lon
        % e->in_usa;
    }
    ofs.close();
    Cons::P(boost::format("created file %s %d") % fn % boost::filesystem::file_size(fn));
    exit(0);
  }

  void Load() {
    _LoadOps();
    //_TestInUsa();
  }

  void FreeMem() {
    if (_entries.size() == 0)
      return;

    Cons::MT _("Freeing ops ...");
    for (auto e: _entries)
      delete e;
  }
}
