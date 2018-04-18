#include <fstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "conf.h"
#include "cons.h"
#include "stat.h"
#include "util.h"
#include "utility-curve.h"
#include "youtube-access.h"

using namespace std;


namespace UtilityCurves {
  namespace bf = boost::filesystem;

  // map<filename(CO_id), LRU_utility_curve>
  map<int, map<long, long>* > _fn_uc;

  // The sum of the max lru cache size from the utility curves.
  //   The value will be the budget upperbound, beyond which won't give you any more benefit.
  long _sum_max_lru_cache_size = 0;

  void _Load(int co_id, const string& dn, map<long, long>* uc);
  void _MakeConvex(map<long, long>*& uc, int co_id);


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

    int max_co_id = atoi(Conf::Get("max_co_id").c_str());

    // const std::map<int, std::vector<std::string>* >& CoAccesses();
    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;
      if (max_co_id != -1 && max_co_id < co_id) {
        Cons::P(boost::format("Passed max_co_id %d. Stop loading") % max_co_id);
        break;
      }

      // Utility curve
      map<long, long>* uc = new map<long, long>();
      _Load(co_id, dn, uc);
      if (boost::algorithm::to_lower_copy(Conf::Get("convert_utility_curves_to_convex")) == "true")
        _MakeConvex(uc, co_id);
      _fn_uc.emplace(co_id, uc);
    }
    Cons::P(boost::format("Loaded %d utility curves.") % _fn_uc.size());

    {
      // This will be the budget upperbound, beyond which won't give you more benefit.
      _sum_max_lru_cache_size = 0;
      for (auto i: _fn_uc) {
        auto uc = i.second;
        if (uc->size() == 0)
          continue;
        _sum_max_lru_cache_size += uc->rbegin()->first;
      }
      Cons::P(boost::format("_sum_max_lru_cache_size=%d") % _sum_max_lru_cache_size);
    }

    // CDF of max LRU cache size
    if (false) {
      vector<long> max_cache_sizes;
      for (auto i: _fn_uc)
        max_cache_sizes.push_back(i.second->rbegin()->first);
      string fn_cdf = str(boost::format("%s/cdf-max-lru-cache-size-per-CO-from-utility-curves") % Conf::DnOut());
      Stat::Gen<long>(max_cache_sizes, fn_cdf);
    }

    // Print the curves
    if (false) {
      for (auto i: _fn_uc) {
        Cons::P(boost::format("%d") % i.first);
        for (auto j: *(i.second)) {
          Cons::P(boost::format("  %ld %ld") % j.first % j.second);
        }
      }
    }
  }


  void _Load(int co_id, const string& dn, map<long, long>* uc) {
    string fn = str(boost::format("%s/%d") % dn % co_id);
    //Cons::P(boost::format("%d %s") % co_id % fn);

    if (! bf::exists(fn))
      THROW(boost::format("Unexpected [%s]") % fn);
    ifstream ifs(fn);
    string line;
    int parse_state = 0;  // 0: uninitialized, 1: LRU, 2: LFU, 3: Optimal
    //long cache_size_prev = -1;
    long bytes_hit_prev = 0;
    //bool stored_last = false;
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

        if (bytes_hit_prev == bytes_hit) {
          // Skip intermediate points when bytes_hit doesn't change to save space
          //stored_last = false;
        } else if (bytes_hit_prev < bytes_hit) {
          // Don't store the prev point before a change. We don't want a step function. We want slopes.
          //if (! stored_last)
          //  uc->emplace(cache_size_prev, bytes_hit_prev);
          uc->emplace(cache_size, bytes_hit);
          //stored_last = true;
        } else {
          THROW("Unexpected");
        }

        //cache_size_prev = cache_size;
        bytes_hit_prev = bytes_hit;
      } else {
        break;
      }
    }

    // You don't need to store the last point when its bytes_hit is the same. Otherwise you get a loose max cache space.
    //if (! stored_last)
    //  uc->emplace(cache_size_prev, bytes_hit_prev);
  }


  // Returns the number of points deleted
  void _MakeConvex(map<long, long>*& uc, int co_id) {
    // x: cache space
    // y: utility (bytes served)

    map<long, long>* uc_new = new map<long, long>();

    int num_points_deleted = 0;
    int num_points_concave = 0;

    for (auto i: *uc) {
      long x = i.first;
      long y = i.second;

      //TRACE << boost::format("x=%d y=%d\n") % x % y;

      if (uc_new->size() == 0) {
        if (x == 0 || y == 0)
          THROW("Unexpected");
        uc_new->emplace(x, y);
        continue;
      }

      // Make sure that the next point is making a right turn
      //   If the next point is along the same straight line or making a left turn, delete the previous point
      while (true) {
        auto it = uc_new->rbegin();
        if (it == uc_new->rend())
          break;

        // p1(x1, y1): prev point
        // p2(x2, y2): prev prev point
        long x1 = it->first;
        long y1 = it->second;
        if (x == x1 || y == y1)
          THROW("Unexpected");

        long x2;
        long y2;

        it ++;
        if (it == uc_new->rend()) {
          x2 = 0;
          y2 = 0;
        } else {
          x2 = it->first;
          y2 = it->second;
        }

        // Test which way the line p2-p1-p0 is making a turn
        //   https://www.geeksforgeeks.org/orientation-3-ordered-points
        //   We use long to avoid floating point errors. We assume the intermediate values fit in long.
        //     double val = double(y1 - y2) * (x - x1) - double(x1 - x2) * (y - y1);
        //     long val = (y1 - y2) * (x - x1) - (x1 - x2) * (y - y1);
        double val = double(y1 - y2) * (x - x1) - double(x1 - x2) * (y - y1);
        //Cons::P(boost::format("%d %d|%d %d|%d %d|val=%f") % x2 % y2 % x1 % y1 % x % y % val);

        if (val <= 0.0) {
          // Drop the last (prev) point
          uc_new->erase((++ (uc_new->rbegin())).base());
          num_points_deleted ++;
          if (val < 0.0)
            num_points_concave ++;
        } else {
          // The line segment is making a right turn
          break;
        }
      }
      uc_new->emplace(x, y);
    }

    if (0 < num_points_concave) {
      if (false) {
        Cons::P(boost::format("co_id=%d num_points_deleted=%d num_points_concave=%d") % co_id % num_points_deleted % num_points_concave);
        vector<string> s;
        long x_prev = 0;
        long y_prev = 0;
        for (auto i: *uc) {
          long x = i.first;
          long y = i.second;
          double slope = double(y - y_prev) / (x - x_prev);
          s.push_back(str(boost::format("%d %d %f") % x % y % slope));
          x_prev = x;
          y_prev = y;
        }
        vector<string> s1;
        x_prev = 0;
        y_prev = 0;
        for (auto i: *uc_new) {
          long x = i.first;
          long y = i.second;
          double slope = double(y - y_prev) / (x - x_prev);
          s1.push_back(str(boost::format("%d %d %f") % x % y % slope));
          x_prev = x;
          y_prev = y;
        }
        Cons::P(boost::format("  %s") % boost::join(s, "|"));
        Cons::P(boost::format("  %s") % boost::join(s1, "|"));
      }
      delete uc;
      uc = uc_new;
    } else {
      delete uc_new;
    }
  }


  void FreeMem() {
    for (auto i: _fn_uc) {
      delete i.second;
    }
    _fn_uc.clear();
  }


  long SumMaxLruCacheSize() {
    return _sum_max_lru_cache_size;
  }


  const map<int, map<long, long>* >& Get() {
    return _fn_uc;
  }
}
