#include <fstream>
#include <map>
#include <typeinfo>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem.hpp>

#include "conf.h"
#include "cons.h"
#include "gen-plot-data.h"
#include "stat.h"
#include "youtube-data.h"

using namespace std;

namespace GenPlotData {

  void Run() {
    YoutubeData::Load();

    //Cons::MT _(boost::format("Loading YouTube workload from file %s ...") % fn);

    // To reduce the number of boost calls, we bin the requests first by date
    // map<date, cnt>
    map<string, int> date_cnt;
    for (auto op: YoutubeData::Entries()) {
      const string& date0 = op->created_at.substr(0, 10);
      auto it = date_cnt.find(date0);
      if (it == date_cnt.end()) {
        date_cnt[date0] = 1;
      } else {
        it->second ++;
      }
    }

    const std::locale fmt0(std::locale::classic(), new boost::gregorian::date_input_facet("%Y-%m-%d"));
    map<boost::gregorian::greg_weekday, vector<int> > dow_cnt;
    for (auto i: date_cnt) {
      const auto& date0 = i.first;
      std::istringstream is(date0);
      is.imbue(fmt0);
      boost::gregorian::date date1;
      is >> date1;

      auto dow = date1.day_of_week();
      auto it = dow_cnt.find(dow);
      if (it == dow_cnt.end()) {
        dow_cnt[dow] = vector<int>();
      }
      dow_cnt[dow].push_back(i.second);
    }

    string fn_out = Conf::Get("out_fn");
    string dn_out = boost::filesystem::path(__FILE__).parent_path().string();
    boost::filesystem::create_directories(dn_out);

    ofstream ofs(fn_out);
    string fmt = "%3s %7.2f %4f %4f %4f %4f %4f";
    ofs << Util::BuildHeader(fmt, "Dow avg min 25p 50p 75p max") << "\n";

    int weekly_max = 0;
    for (auto i: dow_cnt) {
      auto dow = i.first;
      auto cnts = i.second;

      Stat::Result<int> r = Stat::Gen<int>(cnts);
      ofs << boost::format(fmt + "\n") % dow % r.avg % r.min % r._25p % r._50p % r._75p % r.max;
      weekly_max = max(weekly_max, r.max);
    }

    ofs << "# weekly_max=" << weekly_max << "\n";
    ofs.close();
    Cons::P(boost::format("Created %s %d") % fn_out % boost::filesystem::file_size(fn_out));
  }

};
