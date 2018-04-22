#include <signal.h>
#include <unistd.h>
#include <iostream>
#include "conf.h"
#include "cons.h"
#include "central-office.h"
#include "util.h"
#include "utility-curve.h"
#include "youtube-access.h"
#include "world.h"

using namespace std;


void on_signal(int sig) {
  cout << boost::format("\nGot a signal: %d\n%s\n") % sig % Util::Indent(Util::StackTrace(1), 2);
  exit(1);
}


int main(int argc, char* argv[]) {
  try {
    signal(SIGSEGV, on_signal);
    signal(SIGINT, on_signal);

    Conf::Init(argc, argv);

    // TODO: Add data access latency simulation

    // Load YouTube accesses first to avoid loading unnecessary (one-hit per CO) utility curves.
    YoutubeAccess::Load();

    UtilityCurves::Load();

    string inc_type = Conf::Get("cache_size_increment_type");
    long aggr_cache_size_max = UtilityCurves::SumMaxLruCacheSize();

    const char* fmt = "%8d %8.6f %6d %7d %8d %8d";
    Cons::P(Util::BuildHeader(fmt, "aggr_cache_size hit_ratio hits misses traffic_o2c traffic_c2u"));

    if (inc_type == "exponential") {
      long aggr_cache_size = aggr_cache_size_max;
      while (true) {
        // Allocate all EdgeDCs
        //   TODO: In the constructor each EdgeDC will calculate the distance from itself to the closest origin DC

        // TODO: This can be renamed to EdgeDc
        //CentralOffices::GetAll();

        World w(aggr_cache_size);
        w.PlayWorkload();
        // TODO: move it into the above
        w.ReportStat(fmt);

        if (aggr_cache_size == 0)
          break;
        aggr_cache_size /= 2;
      }
    } else if (inc_type == "linear") {
      for (int i = 0; i < 10; i ++) {
        long aggr_cache_size = aggr_cache_size_max * (i + 1) / 10;

        World w(aggr_cache_size);
        w.PlayWorkload();
        w.ReportStat(fmt);
      }
    } else {
      THROW("Unexpected");
    }

    {
      Cons::MT _("Freeing memory ...");
      YoutubeAccess::FreeMem();
      UtilityCurves::FreeMem();
    }
  } catch (const exception& e) {
    cerr << "Got an exception: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
