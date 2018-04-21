#pragma once

#include <map>
#include <thread>
#include <vector>

#include "cache.h"
#include "conf.h"
#include "cons.h"
#include "edge-dc.h"
#include "util.h"
#include "youtube-access.h"
#include "ub-alloc.h"


template <class T>
class Caches {
  //map<EL(edge_location)_id, EdgeDC<T>* >
  std::map<int, EdgeDC<T>* > _edgedcs;
  const char* _fmt = "%8d %8.6f %6d %7d %8d %8d";


  void _FreeMem() {
    for (auto i: _edgedcs)
      delete i.second;
    _edgedcs.clear();
  }


public:
  Caches() {
  }


  ~Caches() {
    _FreeMem();
  }


  // Allocate caches and replay workload
  void Run(long total_cache_size_max) {
    Cons::P(Util::BuildHeader(_fmt, "total_cache_size hit_ratio hits misses traffic_o2c traffic_c2u"));

    std::string inc_type = Conf::Get("cache_size_increment_type");
    if (inc_type == "exponential") {
      long total_cache_size = total_cache_size_max;
      while (true) {
        //Cons::MT _(boost::format("total_cache_size=%d") % total_cache_size);
        _AllocateCaches(total_cache_size);
        _PlayWorkload();
        _ReportStat(total_cache_size);

        if (total_cache_size == 0)
          break;
        total_cache_size /= 2;
      }
    } else if (inc_type == "linear") {
      for (int i = 0; i < 10; i ++) {
        long total_cache_size = total_cache_size_max * (i + 1) / 10;
        //Cons::MT _(boost::format("total_cache_size=%d") % total_cache_size);
        _AllocateCaches(total_cache_size);
        _PlayWorkload();
        _ReportStat(total_cache_size);
      }
    } else {
      THROW("Unexpected");
    }
  }


  void _AllocateCaches(long total_cache_size) {
    //Cons::MT _("Initializing caches ...");
    _FreeMem();

    std::string p = Conf::Get("placement_stragegy");
    if (p == "uniform") {
      _AllocateCachesUniform(total_cache_size);
    } else if (p == "req_volume_based") {
      _AllocateCachesReqVolBased(total_cache_size);
    } else if (p == "user_based") {
      _AllocateCachesUserBased(total_cache_size);
    } else if (p == "utility_based") {
      _AllocateCachesUtilityBased(total_cache_size);
    } else {
      THROW("Unexpected");
    }
  }


  void _AllocateCachesUniform(long total_cache_size) {
    size_t num_ELs = YoutubeAccess::ElAccesses().size();

    // Some caches have 1 bytes bigger size than the others due to the integer division.
    long cache_size_0 = total_cache_size / num_ELs;
    long cache_size_1 = cache_size_0 + 1;

    // Allocate cache_size_0 MB for the first a caches and cache_size_1 MB for the others
    // a * cache_size_0 + b * cache_size_1 = total_cache_size
    // a + b = num_ELs
    //
    // a * cache_size_0 + (num_ELs - a) * cache_size_1 = total_cache_size
    // a * (cache_size_0 - cache_size_1) + num_ELs * cache_size_1 = total_cache_size
    // a = (total_cache_size - num_ELs * cache_size_1) / (cache_size_0 - cache_size_1)
    // a = num_ELs * cache_size_1 - total_cache_size
    int a = num_ELs * cache_size_1 - total_cache_size;

    long cache_size = cache_size_0;
    int j = 0;
    for (const auto i: YoutubeAccess::ElAccesses()) {
      j ++;
      if (a < j)
        cache_size = cache_size_1;

      //Cons::P(boost::format("%d") % cache_size);

      int el_id = i.first;
      _edgedcs.emplace(el_id, new EdgeDC<T>(cache_size));
    }
  }


  void _AllocateCachesReqVolBased(long total_cache_size) {
    long total_num_reqs = 0;
    for (auto i: YoutubeAccess::ElAccesses())
      total_num_reqs += i.second->NumAccesses();

    std::map<int, long> elid_cachesize;
    long total_allocated = 0;
    for (auto i: YoutubeAccess::ElAccesses()) {
      int el_id = i.first;
      size_t num_reqs = i.second->NumAccesses();
      long s = double(total_cache_size) * num_reqs / total_num_reqs;
      elid_cachesize.emplace(el_id, s);
      total_allocated += s;
    }

    long remainder = total_cache_size - total_allocated;
    //Cons::P(boost::format("total_cache_size=%d total_allocated=%d remainder=%d")
    //    % total_cache_size % total_allocated % remainder);

    //long alloc0 = 0;
    //for (auto& i: elid_cachesize)
    //  alloc0 += i.second;

    long j = 0;
    for (auto& i: elid_cachesize) {
      i.second ++;
      j ++;
      if (j == remainder)
        break;
    }

    //long alloc1 = 0;
    //for (auto& i: elid_cachesize)
    //  alloc1 += i.second;
    //Cons::P(boost::format("alloc0=%d alloc1=%d") % alloc0 % alloc1);

    for (auto i: elid_cachesize) {
      int el_id = i.first;
      long cache_size = i.second;
      _edgedcs.emplace(el_id, new EdgeDC<T>(cache_size));
    }
  }


  void _AllocateCachesUserBased(long total_cache_size) {
    long total_num_users = 0;
    for (auto i: YoutubeAccess::ElAccesses())
      total_num_users += i.second->NumUsers();

    std::map<int, long> elid_cachesize;
    long total_allocated = 0;
    for (auto i: YoutubeAccess::ElAccesses()) {
      int el_id = i.first;
      size_t num_users = i.second->NumUsers();
      long s = double(total_cache_size) * num_users / total_num_users;
      elid_cachesize.emplace(el_id, s);
      total_allocated += s;
    }

    long remainder = total_cache_size - total_allocated;
    //Cons::P(boost::format("total_cache_size=%d total_allocated=%d remainder=%d")
    //    % total_cache_size % total_allocated % remainder);

    //long alloc0 = 0;
    //for (auto& i: elid_cachesize)
    //  alloc0 += i.second;

    long j = 0;
    for (auto& i: elid_cachesize) {
      i.second ++;
      j ++;
      if (j == remainder)
        break;
    }

    //long alloc1 = 0;
    //for (auto& i: elid_cachesize)
    //  alloc1 += i.second;
    //Cons::P(boost::format("alloc0=%d alloc1=%d") % alloc0 % alloc1);

    for (auto i: elid_cachesize) {
      int el_id = i.first;
      long cache_size = i.second;
      _edgedcs.emplace(el_id, new EdgeDC<T>(cache_size));
    }
  }


  void _AllocateCachesUtilityBased(long total_cache_size) {
    //Cons::MT _("Allocate cache space using utility curves ...");

    std::map<int, long> elid_cachesize;
    UbAlloc::Calc(total_cache_size, elid_cachesize);

    for (auto i: elid_cachesize) {
      int el_id = i.first;
      long cache_size = i.second;
      _edgedcs.emplace(el_id, new EdgeDC<T>(cache_size));
    }
  }


  void _PlayWorkload() {
    //Cons::MT _("Playing workload ...");

    int num_threads = Util::NumHwThreads();
    size_t n = YoutubeAccess::ElAccesses().size();

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
    auto it_begin = YoutubeAccess::ElAccesses().begin();

    int i = 0;
    for (; i < a; i ++) {
      auto it_end = it_begin;
      std::advance(it_end, s0);
      threads.push_back(std::thread(__PlayWorkload0, this, it_begin, it_end));
      it_begin = it_end;
    }
    for (; i < num_threads; i ++) {
      auto it_end = it_begin;
      std::advance(it_end, s1);
      threads.push_back(std::thread(__PlayWorkload0, this, it_begin, it_end));
      it_begin = it_end;
    }
    //Cons::P(boost::format("Running %d threads") % threads.size());
    for (auto& t: threads)
      t.join();

    // You don't need to gather the results here. Each thread updates the Cache instances.
  }


  static void __PlayWorkload0(
      Caches<T>* c,
      typename std::map<int, YoutubeAccess::Accesses* >::const_iterator it_begin,
      typename std::map<int, YoutubeAccess::Accesses* >::const_iterator it_end) {
    c->__PlayWorkload1(it_begin, it_end);
  }


  void __PlayWorkload1(
      typename std::map<int, YoutubeAccess::Accesses* >::const_iterator it_begin,
      typename std::map<int, YoutubeAccess::Accesses* >::const_iterator it_end) {
    for (auto it = it_begin; it != it_end; it ++) {
      int el_id = it->first;
      auto it1 = _edgedcs.find(el_id);
      if (it1 == _edgedcs.end())
        THROW(boost::format("Unexpected. el_id=%d") % el_id);
      EdgeDC<T>* edc = it1->second;

      const YoutubeAccess::Accesses* acc = it->second;

      // it->second is of tyep vector<T>*
      for (const auto& item_key: *(acc->ObjIds())) {
        // Item size is 50 MB for every video: an wild guess of the average YouTube video size downloaded.
        const long item_size = 50;

        if (edc->Get(item_key)) {
          // The item is served (downloaded) from the cache
        } else {
          // The item is not in the cache.

          // Fetch the data item from the origin to the edge cache
          edc->FetchFromOrigin(item_size);

          // Put the item in the cache.
          //   The operation can fail when there isn't enough space for the new item even after evicting existing items.
          //     For example when the cache size is 0.
          //   We don't do anything about the failed operation for now.
          edc->Put(item_key, item_size);
        }
        edc->ServeDataToUser(item_size);
      }
    }
  }


  //void _ShowStatPerCache() {
  //  std::string fmt = "%3d %3d %4d %3d";
  //  std::string header = Util::BuildHeader(fmt, "el_id cache_hits cache_misses num_items_in_cache");
  //  Cons::P(header);
  //  for (const auto i: YoutubeAccess::ElAccesses()) {
  //    int el_id = i.first;
  //    EdgeDC<T>* edc = _edgedcs[el_id];
  //    typename EdgeDC<T>::Stat s = edc->GetStat();
  //    Cons::P(boost::format(fmt) % el_id % s.hits % s.misses % s.num_items);
  //  }
  //}


  void _ReportStat(long total_cache_size) {
    long hits = 0;
    long misses = 0;
    long o2c = 0;
    long c2u = 0;
    for (const auto i: YoutubeAccess::ElAccesses()) {
      int el_id = i.first;
      EdgeDC<T>* edc = _edgedcs[el_id];
      typename EdgeDC<T>::Stat s = edc->GetStat();
      //Cons::P(boost::format(fmt) % el_id % s.hits % s.misses % s.num_items);
      hits += s.cache_stat.hits;
      misses += s.cache_stat.misses;
      o2c += s.traffic_o2c;
      c2u += s.traffic_c2u;
    }

    // 25353550 0.081899 124079 1390937 69546850 75750800
    Cons::P(boost::format(_fmt) % total_cache_size % (double(hits) / (hits + misses)) % hits % misses % o2c % c2u);
  }
};
