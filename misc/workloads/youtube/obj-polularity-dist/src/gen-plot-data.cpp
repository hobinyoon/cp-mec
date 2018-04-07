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

    Cons::MT _("Generating data for plotting ...");

    // map<video_id, cnt>
    map<string, int> vid_cnt;

    // Bin
    for (auto op: YoutubeData::Entries()) {
      const string& oid = op->obj_id;
      auto it = vid_cnt.find(oid);
      if (it == vid_cnt.end()) {
        vid_cnt[oid] = 1;
      } else {
        it->second ++;
      }
    }

    vector<int> cnts;
    for (auto i: vid_cnt)
      cnts.push_back(i.second);

    sort(cnts.begin(), cnts.end());

    // Just cnts. I don't need video IDs here for now.
    const string& fn = Conf::fn_plot_data;
    ofstream ofs(fn);

    int x = 1;
    int prev_y = -1;
    int x_last_printed = 0;
    for (auto it = cnts.rbegin(); it != cnts.rend(); it ++) {
      int y = *it;

      if (prev_y != y) {
        if ((x_last_printed + 1) < x) {
          ofs << boost::format("%d %d\n") % (x-1) % prev_y;
        }

        ofs << boost::format("%d %d\n") % x % y;
        x_last_printed = x;
      }

      prev_y = y;
      x ++;
    }

    // Print the last point
    ofs << boost::format("%d %d\n") % x % prev_y;

    ofs.close();
    Cons::P(boost::format("Created %s %d") % fn % boost::filesystem::file_size(fn));
  }

};
