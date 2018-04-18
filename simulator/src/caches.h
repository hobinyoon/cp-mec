#pragma once

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

      Caches::_AllocateCaches(total_cache_size);
      Caches::_PlayWorkload();
      Caches::_ReportStat(total_cache_size);
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

    // This can be parallelized. We'll see.

    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;
      auto it = _caches.find(co_id);
      if (it == _caches.end())
        THROW(boost::format("Unexpected. co_id=%d") % co_id);
      Cache<T>* c = it->second;

      // i.second is of tyep vector<T>*
      for (const auto& item_key: *i.second) {
        if (c->Get(item_key)) {
          // The item is served (downloaded) from the cache
        } else {
          // The item is not in the cache.
          //   TODO: Fetch from the origin.
          //   Put the item in the cache. For now, we don't care if the Put() succeeds or not (due to a 0-size cache).
          // Item size is 50 MB for every video. An wild guess of the average YouTube video size downloaded.
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
