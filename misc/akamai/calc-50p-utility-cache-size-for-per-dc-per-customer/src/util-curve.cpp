#include <mutex>
#include <set>
#include <string>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "cons.h"
#include "conf.h"
#include "util.h"
#include "util-curve.h"

using namespace std;

namespace UtilCurve {
  namespace bf = boost::filesystem;

  // Customer ID and DC ID
  struct CDC {
    int c;
    int dc;
    CDC (int c_, int dc_)
      : c(c_), dc(dc_)
    {}

    bool operator < (const CDC& r) const {
      if (c < r.c) return true;
      else if (c > r.c) return false;
      return (dc < r.dc);
    }
  };

  // <customer_dc, cache_size>
  //   cache_size is the cache size that gives you half of the cache hits (utility)
  map<CDC, long> _cdc_cs;

  set<int> _customers;
  string _in_dn;

  int _num_files_completed = 0;
  mutex _cout_lock;


  struct Param {
    map<CDC, long>::iterator it_begin;
    map<CDC, long>::iterator it_end;

    Param(
        map<CDC, long>::iterator b,
        map<CDC, long>::iterator e)
      : it_begin(b), it_end(e)
    {}
  };

  void _GetCustomers();
  void _GetDCs();
  void _GetHalfMaxCacheSize();
  void _UcMaxCacheSize(Param* p);
  void _GenOutFile();


  void GetHalfMaxCacheSize() {
    _in_dn = Conf::Get("dn_util_curves");

    _GetCustomers();
    _GetDCs();
    _GetHalfMaxCacheSize();
    _GenOutFile();
  }

  void _GetCustomers() {
    for (auto i = bf::directory_iterator(_in_dn); i != bf::directory_iterator(); i++) {
      auto p = i->path();
      string s = p.filename().string();

      if (s.size() == 0)
        continue;
      if (! isdigit(s[0]))
        continue;

      //Cons::P(p.filename().string());
      _customers.insert(stoi(p.filename().string()));
      break;
    }
    //Cons::P(boost::format("Found %d customers") % _customers.size());
  }


  void _GetDCs() {
    for (auto c_id: _customers) {
      string dn = str(boost::format("%s/%d") % _in_dn % c_id);
      for (auto i = bf::directory_iterator(dn); i != bf::directory_iterator(); i++) {
        auto p = i->path();
        string s = p.filename().string();

        if (s.size() == 0)
          continue;
        if (! isdigit(s[0]))
          continue;

        int dc_id = stoi(p.filename().string());
        _cdc_cs.emplace(CDC(c_id, dc_id), 0);
      }
    }
    Cons::P(boost::format("Found %d customer-DC(s)") % _cdc_cs.size());
  }


  void _GetHalfMaxCacheSize() {
    int num_threads = Util::NumHwThreads();
    Cons::P(boost::format("Found %d HW threads") % num_threads);
    size_t n = _cdc_cs.size();

    // First few threads may have 1 more work items than the other threads due to the integer devision rounding.
    long s1 = n / num_threads;
    long s0 = s1 + 1;

    // Allocate s0 work items for the first a threads and s1 items for the next b threads
    //   a * s0 + b * s1 = n
    //   a + b = num_threads
    //
    //   a * s0 + (num_threads - a) * s1 = n
    //   a * (s0 - s1) + num_threads * s1 = n
    //   a = (n - num_threads * s1) / (s0 - s1)
    //   a = n - num_threads * s1
    int a = n - num_threads * s1;
    //Cons::P(boost::format("n=%d num_threads=%d s0=%d s1=%d a=%d") % n % num_threads % s0 % s1 % a);

    std::vector<std::thread> threads;

    auto it_begin = _cdc_cs.begin();

    map<int, Param*> tid_param;

    int i = 0;
    for (; i < a; i ++) {
      auto it_end = it_begin;
      std::advance(it_end, s0);
      Param* p = new Param(it_begin, it_end);
      tid_param.emplace(i, p);
      threads.push_back(std::thread(&UtilCurve::_UcMaxCacheSize, p));
      it_begin = it_end;
    }
    for (; i < num_threads; i ++) {
      auto it_end = it_begin;
      std::advance(it_end, s1);
      Param* p = new Param(it_begin, it_end);
      tid_param.emplace(i, p);
      threads.push_back(std::thread(&UtilCurve::_UcMaxCacheSize, p));
      it_begin = it_end;
    }
    //Cons::P(boost::format("Running %d threads") % threads.size());
    for (auto& t: threads)
      t.join();
    {
      lock_guard<mutex> _(_cout_lock);
      Cons::ClearLine();
      Cons::P(boost::format("%d/%d files completed") % _num_files_completed % _cdc_cs.size());
    }

    for (auto i: tid_param)
      delete i.second;

    //for (auto i: _cdc_cs) {
    //  const auto& cdc = i.first;
    //  long cache_size = i.second;
    //  Cons::P(boost::format("%d %d %d") % cdc.c % cdc.dc % cache_size);
    //}
  }


  struct CsUtil {
    long cache_size;
    long bytes_hit;
    CsUtil(long c, long b)
      : cache_size(c), bytes_hit(b)
    {}
  };

  size_t _FindCacheSizeWithHit(const vector<CsUtil>& cs_util, long target_bytes_hit, size_t l, size_t r);


  void _UcMaxCacheSize(Param* p) {
    for (auto it = p->it_begin; it != p->it_end; it ++) {
      int c_id = it->first.c;
      int dc_id = it->first.dc;

      // Calculate the cache size that gives you the half of the maximum utility
      string fn = str(boost::format("%s/%d/%d") % _in_dn % c_id % dc_id);
      ifstream ifs(fn);
      string line;
      int parse_state = 0;  // 0: uninitialized, 1: LRU, 2: LFU, 3: Optimal
      long bytes_hit_prev = 0;
      //long max_cache_size = 0;
      long max_bytes_hit = 0;
      vector<CsUtil> cs_util;

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

          long cache_size = stol(t[0]);
          long bytes_hit = stol(t[1]);

          if (bytes_hit_prev == bytes_hit) {
          } else if (bytes_hit_prev < bytes_hit) {
            //max_cache_size = cache_size;
            max_bytes_hit = bytes_hit;
            cs_util.push_back(CsUtil(cache_size, bytes_hit));
          } else {
            THROW("Unexpected");
          }

          bytes_hit_prev = bytes_hit;
        } else {
          break;
        }
      }
      //Cons::P(boost::format("max_bytes_hit=%d") % max_bytes_hit);

      if (cs_util.size() == 0) {
        it->second = 0;
      } else {
        size_t idx = _FindCacheSizeWithHit(cs_util, max_bytes_hit / 2, 0, cs_util.size());
        //Cons::P(boost::format("idx=%d cache_size=%d max_bytes_hit=%d") % idx % cs_util[idx].cache_size % cs_util[idx].bytes_hit);
        it->second = cs_util[idx].cache_size;
      }

      // A quick implementation. I know it's not a good design.
      {
        lock_guard<mutex> _(_cout_lock);
        _num_files_completed ++;
        Cons::ClearLine();
        Cons::Pnnl(boost::format("%d/%d files completed. halfhit_cachesize=%d")
            % _num_files_completed % _cdc_cs.size() % it->second);
      }
    }
  }


  // Returns the index of cs_util
  //   Range in [l, r)
  size_t _FindCacheSizeWithHit(const vector<CsUtil>& cs_util, long target_bytes_hit, size_t l, size_t r) {
    // Do a binary search
    //Cons::P(boost::format("l=%d r=%d") % l % r);

    if (l == r) {
      return l;
    }

    size_t m = (l + r) / 2;
    if (l == m) {
      return l;
    }

    if (m == r)
      THROW(boost::format("Unexpected %d") % l);

    long bytes_hit = cs_util[m].bytes_hit;
    if (bytes_hit == target_bytes_hit) {
      return m;
    } else if (bytes_hit < target_bytes_hit) {
      return _FindCacheSizeWithHit(cs_util, target_bytes_hit, m, r);
    } else {
      return _FindCacheSizeWithHit(cs_util, target_bytes_hit, l, m);
    }
  }


  void _GenOutFile() {
    string fn = str(boost::format("%s/customer-dc-cachesize") % Conf::DnOut());

    ofstream ofs(fn);
    ofs << "# customer dc cache_size\n";

    for (auto& i: _cdc_cs) {
      int c_id = i.first.c;
      int dc_id = i.first.dc;
      long cache_size = i.second;
      ofs << boost::format("%d %d %d\n") % c_id % dc_id % cache_size;
    }
    ofs.close();

    Cons::P(boost::format("Created %s %d") % fn % boost::filesystem::file_size(fn));
  }
};
