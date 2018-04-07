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
	string dn_plot_data;
	string fn_plot_data;

	YAML::Node _yaml_root;

	void _LoadYaml() {
		string fn = str(boost::format("%s/../config.yaml") % boost::filesystem::path(__FILE__).parent_path().string());
		_yaml_root = YAML::LoadFile("config.yaml");
	}

	namespace po = boost::program_options;

	template<class T>
	void __EditYaml(const string& key, po::variables_map& vm) {
		if (vm.count(key) != 1)
			return;
		T v = vm[key].as<T>();
		static const auto sep = boost::is_any_of(".");
		vector<string> tokens;
		boost::split(tokens, key, sep, boost::token_compress_on);
		// Had to use a pointer to traverse the tree. Otherwise, the tree gets
		// messed up.
		YAML::Node* n = &_yaml_root;
		for (string t: tokens) {
			YAML::Node n1 = (*n)[t];
			n = &n1;
		}
		*n = v;
		//Cons::P(Desc());
	}

	void _ParseArgs(int argc, char* argv[]) {
		po::options_description od("Allowed options");
		od.add_options()
			("youtube_workload",
			 po::value<string>()->default_value(Conf::GetString("youtube_workload")))
			("help", "show help message")
			;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(od).run(), vm);
		po::notify(vm);

		if (vm.count("help") > 0) {
			// well... this doesn't show boolean as string.
			cout << std::boolalpha;
			cout << od << "\n";
			exit(0);
		}

		__EditYaml<string>("youtube_workload", vm);
	}

	void Init(int argc, char* argv[]) {
		Cons::MT _("Init ...");

		_LoadYaml();
		_ParseArgs(argc, argv);

		dn_plot_data = str(boost::format("%s/../.output")
				% boost::filesystem::path(__FILE__).parent_path().string());
		boost::filesystem::create_directories(dn_plot_data);
		fn_plot_data = str(boost::format("%s/%s-obj-pop-dist")
				% dn_plot_data
				% boost::filesystem::path(GetString("youtube_workload")).filename().string()
        );

		Cons::P(boost::format("dn_plot_data=%s") % dn_plot_data);
		Cons::P(boost::format("fn_plot_data=%s") % fn_plot_data);
	}

	YAML::Node Get(const std::string& k) {
		return _yaml_root[k];
	}

	string GetString(const std::string& k) {
		return _yaml_root[k].as<string>();
	}

	const string Desc() {
		YAML::Emitter emitter;
		emitter << _yaml_root;
		if (! emitter.good())
			THROW("Unexpected");
		return emitter.c_str();
	}
};
