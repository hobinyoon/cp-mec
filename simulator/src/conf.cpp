#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "conf.h"
#include "cons.h"
#include "util.h"


using namespace std;

namespace Conf {
  YAML::Node _yaml_root;

  string _dn_out;

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
      ("video_accesses_by_COs",
       po::value<string>()->default_value(GetStr("video_accesses_by_COs")))
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

    //__EditYaml<size_t>("micro_edge_dc.num_deployed", vm);
    __EditYaml<string>("video_accesses_by_COs", vm);
  }

  void Init(int argc, char* argv[]) {
    //_exp_id = Util::CurDateTime();
    _LoadYaml();
    _ParseArgs(argc, argv);
    //Cons::P(boost::format("Exp ID: %s") % _exp_id);

    _dn_out = str(boost::format("%s/../.output") % boost::filesystem::path(__FILE__).parent_path().string());
    boost::filesystem::create_directories(_dn_out);
  }

  YAML::Node Get(const std::string& k) {
    return _yaml_root[k];
  }

  string GetStr(const std::string& k) {
    return _yaml_root[k].as<string>();
  }

  string GetFn(const std::string& k) {
    // Use boost::regex. C++11 regex works from 4.9. Ubuntu 14.04 has g++ 4.8.4.
    //   http://stackoverflow.com/questions/8060025/is-this-c11-regex-error-me-or-the-compiler
    return boost::regex_replace(
        GetStr(k)
        , boost::regex("~")
        , Util::HomeDir());
  }

  const string Desc() {
    YAML::Emitter emitter;
    emitter << _yaml_root;
    if (! emitter.good())
      THROW("Unexpected");
    return emitter.c_str();
  }

  const string DnOut() {
    return _dn_out;
  }

  // void StoreConf() {
  //   boost::filesystem::create_directories(OutputDir());
  //   const string fn = str(boost::format("%s/conf") % Conf::OutputDir());
  //   ofstream ofs(fn);
  //   if (! ofs.is_open())
  //     THROW(boost::format("Unable to open file %s") % fn);
  //   ofs << Desc() << "\n";
  // }
};
