#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "central-office.h"
#include "conf.h"
#include "cons.h"
#include "util.h"
#include "utility-curve.h"

using namespace std;

namespace UtilityCurves {
  namespace bf = boost::filesystem;

  // map<fn, LRU_utility_curve>
  map<string, map<long, long>* > _fn_uc;

  void Load() {
    string fn = Conf::GetFn("utility_curves");
    if (! bf::exists(fn))
      THROW("Unexpected");

    Cons::MT _(boost::format("Loading utility curves from %s ...") % fn);

    if (! boost::algorithm::ends_with(fn, ".tar.7z"))
      THROW("Unexpected");

    string dn = fn.substr(0, fn.size() - 7);

    if (! bf::exists(dn)) {
      string dn_p = bf::path(dn).parent_path().string();

      string dn = bf::path(fn).parent_path().string();
      string fn1 = bf::path(fn).filename().string();
      string cmd = str(boost::format("cd %s && 7z e -so %s | tar xf -") % dn_p % fn);
      Util::RunSubprocess(cmd);
    }
    if (! bf::exists(dn))
      THROW("Unexpected");

    // Start with LRU
    //   TODO: think about if you'd need the others too

    int i = 0;

    bf::directory_iterator end_itr; // default construction yields past-the-end
    for (bf::directory_iterator it(dn); it != end_itr; ++ it) {
      string fn = it->path().string();
      //Cons::P(fn);
      ifstream ifs(fn);
      string line;
      map<long, long>* uc = new map<long, long>();
      int parse_state = 0;  // 0: uninitialized, 1: LRU, 2: LFU, 3: Optimal
      while (getline(ifs, line)) {
        if (line.size() == 0)
          continue;

        if (line == "LRU") {
          parse_state = 1;
          continue;
        } else if (line == "LFU") {
          parse_state = 2;
          continue;
        } else if (line == "Optimal") {
          parse_state = 3;
          continue;
        }

        if (parse_state == 1) {
          // 50: 50
          //Cons::P(line);
          static const auto sep = boost::is_any_of(": ");
          vector<string> t;
          boost::split(t, line, sep, boost::token_compress_on);
          if (t.size() != 2)
            THROW(boost::format("Unexpected %s [%s]") % fn % line);

          long cache_size = atol(t[0].c_str());
          long bytes_hit = atol(t[1].c_str());
          uc->emplace(cache_size, bytes_hit);
        } else {
          break;
        }
      }

      string fn0 = it->path().filename().string();
      _fn_uc.emplace(fn0, uc);

      if (i == 5)
        break;
      i ++;
    }

    for (auto i: _fn_uc) {
      Cons::P(boost::format("%s") % i.first);
      for (auto j: *(i.second)) {
        Cons::P(boost::format("  %ld %ld") % j.first % j.second);
      }
    }
  }

  void FreeMem() {
    for (auto i: _fn_uc) {
      delete i.second;
    }
    _fn_uc.clear();
  }
}
