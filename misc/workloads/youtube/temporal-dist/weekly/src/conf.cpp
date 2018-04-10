#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "conf.h"
#include "cons.h"
#include "util.h"


using namespace std;

namespace Conf {
	namespace po = boost::program_options;
  po::variables_map _vm;

	void _ParseArgs(int argc, char* argv[]) {
		po::options_description od("Allowed options");
		od.add_options()
			("youtube_workload", po::value<string>())
			("out_fn", po::value<string>())
			("help", "show help message")
			;

		po::store(po::command_line_parser(argc, argv).options(od).run(), _vm);
		po::notify(_vm);

		if (_vm.count("help") > 0) {
			// well... this doesn't show boolean as string.
			cout << std::boolalpha;
			cout << od << "\n";
			exit(0);
		}
	}

	void Init(int argc, char* argv[]) {
		Cons::MT _("Init ...");
		_ParseArgs(argc, argv);
	}

const string Get(const std::string& k) {
		return _vm[k].as<string>();
	}
};
