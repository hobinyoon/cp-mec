#include <fstream>
#include <map>
#include <boost/filesystem.hpp>

#include "conf.h"
#include "cons.h"
#include "gen-plot-data.h"
#include "youtube-data.h"

using namespace std;

namespace GenPlotData {

  void Run() {
    YoutubeData::Load();

    //Cons::MT _(boost::format("Loading YouTube workload from file %s ...") % fn);

    // map<date, cnt>
    map<string, int> date_cnt;

    // Bin
    for (auto op: YoutubeData::Entries()) {
      const string& date = op->created_at.substr(0, 10);
      auto it = date_cnt.find(date);
      if (it == date_cnt.end()) {
        date_cnt[date] = 1;
      } else {
        it->second ++;
      }
    }

    int daily_max = 0;
    for (auto i: date_cnt)
      daily_max = max(daily_max, i.second);

    string fn_out = Conf::Get("out_fn");
    string dn_out = boost::filesystem::path(__FILE__).parent_path().string();
    boost::filesystem::create_directories(dn_out);

    ofstream ofs(fn_out);
    ofs << "# daily_max=" << daily_max << "\n";
    ofs << "#\n";
    ofs << "# date cnt\n";
    for (auto i: date_cnt)
      ofs << boost::format("%s %d\n") % i.first % i.second;
    ofs.close();
    Cons::P(boost::format("Created %s %d") % fn_out % boost::filesystem::file_size(fn_out));
  }

};
