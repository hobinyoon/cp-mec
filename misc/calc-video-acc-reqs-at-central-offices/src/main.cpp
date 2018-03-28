#include <signal.h>
#include <unistd.h>
#include <iostream>
#include "conf.h"
#include "cons.h"
//#include "latency.h"
#include "central-office.h"
#include "util.h"
//#include "workload-player.h"
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

    // TODO: Read the workload and map each video access request to a closest central office
    // I don't think it takes too long. The computation can be done in parallel.

		CentralOffices::Init();

		//YoutubeAccess::Load();
		//WorkloadPlayer::MapAccessToCentraloffice();
		//YoutubeAccess::FreeMem();
	} catch (const exception& e) {
		cerr << "Got an exception: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
