#include <signal.h>
#include <unistd.h>
#include <iostream>
#include "caches.h"
#include "conf.h"
#include "cons.h"
#include "central-office.h"
#include "util.h"
#include "utility-curve.h"
#include "youtube-access.h"

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

    // TODO: what do you want to measure?
    //  Cache hit ratio at each CO.
    //    Could plot a CDF of them or an aggregate hit ratio
    //  Data access latency

    // Load YouTube accesses first to avoid loading unnecessary (one-hit per CO) utility curves.
    YoutubeAccess::Load();

    UtilityCurves::Load();

    Caches::Init();
    Caches::PlayWorkload();
    Caches::ShowStat();

    {
      Cons::MT _("Freeing memory ...");
      Caches::FreeMem();
      YoutubeAccess::FreeMem();
      UtilityCurves::FreeMem();
    }
  } catch (const exception& e) {
    cerr << "Got an exception: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
