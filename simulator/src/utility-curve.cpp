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
      string cmd = str(boost::format("cd %s && 7z e -so %s | tar xf -") % dn_p % fn);
      Util::RunSubprocess(cmd);
    }
    if (! bf::exists(dn))
      THROW("Unexpected");

    // Start with LRU
    //   TODO: think about if you'd need the others too

    map<int, bf::path> coid_path;
    bf::directory_iterator end_itr; // default construction yields past-the-end
    for (bf::directory_iterator it(dn); it != end_itr; ++ it) {
      int co_id = atoi(it->path().filename().string().c_str());
      coid_path.emplace(co_id, it->path());
    }
    //for (auto i: coid_path)
    //  Cons::P(boost::format("%d %s") % i.first % i.second.string());

    int max_co_id = atoi(Conf::GetStr("max_co_id").c_str());
    for (auto i: coid_path) {
      int co_id = i.first;
      if (max_co_id != -1 && max_co_id < co_id) {
        Cons::P(boost::format("Passed max_co_id %d. Stop loading") % max_co_id);
        break;
      }

      const bf::path& p = i.second;
      const string& fn = p.string();
      //Cons::P(boost::format("%d %s") % co_id % fn);

      ifstream ifs(fn);
      string line;
      // Utility curve
      map<long, long>* uc = new map<long, long>();
      int parse_state = 0;  // 0: uninitialized, 1: LRU, 2: LFU, 3: Optimal
      long cache_size_prev = -1;
      long bytes_hit_prev = -1;
      bool stored_last = false;
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

          if (cache_size_prev == -1) {
            uc->emplace(cache_size, bytes_hit);
            stored_last = true;
          } else {
            if (bytes_hit_prev == bytes_hit) {
              // Skip intermediate points when bytes_hit doesn't change to save space
              stored_last = false;
            } else {
              if (! stored_last)
                uc->emplace(cache_size_prev, bytes_hit_prev);
              uc->emplace(cache_size, bytes_hit);
              stored_last = true;
            }
          }

          cache_size_prev = cache_size;
          bytes_hit_prev = bytes_hit;
        } else {
          break;
        }
      }

      if (! stored_last)
        uc->emplace(cache_size_prev, bytes_hit_prev);

      string fn0 = p.filename().string();
      _fn_uc.emplace(fn0, uc);
    }
    Cons::P(boost::format("Loaded %d utility curves") % _fn_uc.size());

    if (false) {
      for (auto i: _fn_uc) {
        Cons::P(boost::format("%s") % i.first);
        for (auto j: *(i.second)) {
          Cons::P(boost::format("  %ld %ld") % j.first % j.second);
        }
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
