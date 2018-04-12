#include <fstream>
#include <map>
#include <typeinfo>
#include <vector>

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

    Cons::MT _("Grouping ...");

    // Bin the requests first by their hours
    //   map<ymdh, cnt>
    map<string, int> ymdh_cnt;

    // This grouping doesn't consider the missing values. I don't think it really matters though.
    for (auto op: YoutubeData::Entries()) {
      const string& ymdh = op->created_at.substr(0, 13);
      // 2016-06-01 00:00:04
      // 0123456789012345678
      //Cons::P(ymdh);

      auto it = ymdh_cnt.find(ymdh);
      if (it == ymdh_cnt.end()) {
        ymdh_cnt[ymdh] = 1;
      } else {
        it->second ++;
      }
    }

    map<string, vector<int> > hour_cnt;
    for (auto i: ymdh_cnt) {
      const string hour = i.first.substr(11);

      auto it = hour_cnt.find(hour);
      if (it == hour_cnt.end()) {
        hour_cnt[hour] = vector<int>();
      }
      hour_cnt[hour].push_back(i.second);
    }

    string fn_out = Conf::Get("out_fn");
    string dn_out = boost::filesystem::path(__FILE__).parent_path().string();
    boost::filesystem::create_directories(dn_out);

    ofstream ofs(fn_out);
    string fmt = "%2d %7.2f %4f %4f %4f %4f %4f";
    ofs << Util::BuildHeader(fmt, "hour avg min 25p 50p 75p max") << "\n";

    int hourly_max = 0;
    for (auto i: hour_cnt) {
      int hour = atoi(i.first.c_str());
      auto cnts = i.second;

      Stat::Result<int> r = Stat::Gen<int>(cnts);
      ofs << boost::format(fmt + "\n") % hour % r.avg % r.min % r._25p % r._50p % r._75p % r.max;
      hourly_max = max(hourly_max, r.max);
    }

    ofs << "# hourly_max=" << hourly_max << "\n";
    ofs.close();
    Cons::P(boost::format("Created %s %d") % fn_out % boost::filesystem::file_size(fn_out));
  }

};
