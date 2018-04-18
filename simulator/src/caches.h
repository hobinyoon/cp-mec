#pragma once

#include <map>
#include <thread>
#include <vector>

#include "cache.h"
#include "cons.h"
#include "util.h"
#include "youtube-access.h"
#include "ub-alloc.h"


template <class T>
class Caches {
  //map<cache_id, Cache<T>* >
  // cache_id is CO_id for mobile COs.
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
    for (int i = 0; i < 10; i ++) {
      long total_cache_size = total_cache_size_max * (i + 1) / 10;
      Cons::MT _(boost::format("total_cache_size=%d") % total_cache_size);

      _AllocateCaches(total_cache_size);
      _PlayWorkload();
      _ReportStat(total_cache_size);
    }
  }


  void _AllocateCaches(long total_cache_size) {
    //Cons::MT _("Initializing caches ...");
    _FreeMem();
    //_AllocateCachesUniform(total_cache_size);
    _AllocateCachesUtilityBased(total_cache_size);
  }


  void _AllocateCachesUniform(long total_cache_size) {
    size_t num_COs = YoutubeAccess::CoAccesses().size();

    // Some caches have 1 bytes bigger size than the others due to the integer division.
    long cache_size_0 = total_cache_size / num_COs;
    long cache_size_1 = cache_size_0 + 1;

    // Allocate cache_size_0 MB for the first a caches and cache_size_1 MB for the others
    // a * cache_size_0 + b * cache_size_1 = total_cache_size
    // a + b = num_COs
    //
    // a * cache_size_0 + (num_COs - a) * cache_size_1 = total_cache_size
    // a * (cache_size_0 - cache_size_1) + num_COs * cache_size_1 = total_cache_size
    // a = (total_cache_size - num_COs * cache_size_1) / (cache_size_0 - cache_size_1)
    // a = num_COs * cache_size_1 - total_cache_size
    int a = num_COs * cache_size_1 - total_cache_size;

    long cache_size = cache_size_0;
    int j = 0;
    for (const auto i: YoutubeAccess::CoAccesses()) {
      j ++;
      if (a < j)
        cache_size = cache_size_1;

      //Cons::P(boost::format("%d") % cache_size);
      Cache<T>* c = new Cache<T>(cache_size);

      int co_id = i.first;
      _caches.emplace(co_id, c);
    }
  }


  void _AllocateCachesUtilityBased(long total_cache_size) {
    //Cons::MT _("Allocate cache space using utility curves ...");

    std::map<int, long> coid_cachesize;
    UbAlloc::Calc(total_cache_size, coid_cachesize);

    for (auto i: coid_cachesize) {
      int co_id = i.first;
      long cache_size = i.second;
      _caches.emplace(co_id, new Cache<T>(cache_size));
    }
  }


  void _PlayWorkload() {
    //Cons::MT _("Playing workload ...");

    int num_threads = Util::NumHwThreads();
    size_t n = YoutubeAccess::CoAccesses().size();

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
    auto it_begin = YoutubeAccess::CoAccesses().begin();

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
      typename std::map<int, std::vector<T>* >::const_iterator it_begin,
      typename std::map<int, std::vector<T>* >::const_iterator it_end) {
    c->__PlayWorkload1(it_begin, it_end);
  }


  void __PlayWorkload1(
      typename std::map<int, std::vector<T>* >::const_iterator it_begin,
      typename std::map<int, std::vector<T>* >::const_iterator it_end) {
    for (auto it = it_begin; it != it_end; it ++) {
      int co_id = it->first;
      auto it1 = _caches.find(co_id);
      if (it1 == _caches.end())
        THROW(boost::format("Unexpected. co_id=%d") % co_id);
      Cache<T>* c = it1->second;

      // it->second is of tyep vector<T>*
      for (const auto& item_key: *(it->second)) {
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
    std::string header = Util::BuildHeader(fmt, "co_id cache_hits cache_misses num_items_in_cache");
    Cons::P(header);
    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;
      Cache<T>* c = _caches[co_id];
      typename Cache<T>::Stat s = c->GetStat();
      Cons::P(boost::format(fmt) % co_id % s.hits % s.misses % s.num_items);
    }
  }

  void _ReportStat(long total_cache_size) {
    long hits = 0;
    long misses = 0;
    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;
      Cache<T>* c = _caches[co_id];
      typename Cache<T>::Stat s = c->GetStat();
      //Cons::P(boost::format(fmt) % co_id % s.hits % s.misses % s.num_items);
      hits += s.hits;
      misses += s.misses;
    }

    Cons::P(boost::format("%d %f %d %d") % total_cache_size % (double(hits) / (hits + misses)) % hits % misses);
  }


};
