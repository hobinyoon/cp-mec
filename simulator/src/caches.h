#pragma once

#include <map>
#include <thread>
#include <vector>

#include "cache.h"
#include "conf.h"
#include "cons.h"
#include "util.h"
#include "youtube-access.h"
#include "ub-alloc.h"


template <class T>
class Caches {
  //map<cache_id, Cache<T>* >
  // cache_id is the same as EL_id (edge location ID)
  std::map<int, Cache<T>* > _caches;


  void _FreeMem() {
    for (auto i: _caches)
      delete i.second;
    _caches.clear();
  }


public:
  Caches() {
  }


  ~Caches() {
    _FreeMem();
  }


  // Allocate caches and replay workload
  void Run(long total_cache_size_max) {
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
      Cache<T>* c = new Cache<T>(cache_size);

      int el_id = i.first;
      _caches.emplace(el_id, c);
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
      _caches.emplace(el_id, new Cache<T>(cache_size));
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
      _caches.emplace(el_id, new Cache<T>(cache_size));
    }
  }


  void _AllocateCachesUtilityBased(long total_cache_size) {
    //Cons::MT _("Allocate cache space using utility curves ...");

    std::map<int, long> elid_cachesize;
    UbAlloc::Calc(total_cache_size, elid_cachesize);

    for (auto i: elid_cachesize) {
      int el_id = i.first;
      long cache_size = i.second;
      _caches.emplace(el_id, new Cache<T>(cache_size));
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
      auto it1 = _caches.find(el_id);
      if (it1 == _caches.end())
        THROW(boost::format("Unexpected. el_id=%d") % el_id);
      Cache<T>* c = it1->second;

      const YoutubeAccess::Accesses* acc = it->second;

      // it->second is of tyep vector<T>*
      for (const auto& item_key: *(acc->ObjIds())) {
        if (c->Get(item_key)) {
          // The item is served (downloaded) from the cache
        } else {
          // The item is not in the cache.
          //   TODO: Simulate fetching from the origin for the latency calculation. You can probably do an aggregate latency calculation.
          //   Put the item in the cache. For now, we don't care if the Put() succeeds or not (due to a 0-size cache).
          // Item size is 50 MB for every video: an wild guess of the average YouTube video size downloaded.
          const long item_size = 50;
          c->Put(item_key, item_size);
        }
      }
    }
  }


  void _ShowStatPerCache() {
    std::string fmt = "%3d %3d %4d %3d";
    std::string header = Util::BuildHeader(fmt, "el_id cache_hits cache_misses num_items_in_cache");
    Cons::P(header);
    for (const auto i: YoutubeAccess::ElAccesses()) {
      int el_id = i.first;
      Cache<T>* c = _caches[el_id];
      typename Cache<T>::Stat s = c->GetStat();
      Cons::P(boost::format(fmt) % el_id % s.hits % s.misses % s.num_items);
    }
  }


  void _ReportStat(long total_cache_size) {
    long hits = 0;
    long misses = 0;
    for (const auto i: YoutubeAccess::ElAccesses()) {
      int el_id = i.first;
      Cache<T>* c = _caches[el_id];
      typename Cache<T>::Stat s = c->GetStat();
      //Cons::P(boost::format(fmt) % el_id % s.hits % s.misses % s.num_items);
      hits += s.hits;
      misses += s.misses;
    }

    Cons::P(boost::format("%d %f %d %d") % total_cache_size % (double(hits) / (hits + misses)) % hits % misses);
  }
};
