#include "cache.h"
#include "caches.h"
#include "cons.h"
#include "util.h"
#include "youtube-access.h"

using namespace std;


namespace Caches {
  //map<cache_id, Cache*>
  // cache_id is CO_id for mobile COs.
  map<int, Cache*> _caches;


  void _AllocateCaches(long total_cache_size);
  void _AllocateCachesUniform(long total_cache_size);
  void _PlayWorkload();
  void _ReportStat(long total_cache_size);


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
    FreeMem();
    _AllocateCachesUniform(total_cache_size);
  }


  void _AllocateCachesUniform(long total_cache_size) {
    // Allocate cache
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
      Cache* c = new Cache(cache_size);

      int co_id = i.first;
      _caches.emplace(co_id, c);
    }
  }


  void _PlayWorkload() {
    //Cons::MT _("Playing workload ...");

    // This can be parallelized. We'll see.

    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;
      Cache* c = _caches[co_id];

      // i.second is of tyep vector<string>*
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

  void FreeMem() {
    for (auto i: _caches)
      delete i.second;
    _caches.clear();
  }

  void _ShowStatPerCache() {
    string fmt = "%3d %3d %4d %3d";
    string header = Util::BuildHeader(fmt, "co_id cache_hits cache_misses num_items_in_cache");
    Cons::P(header);
    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;
      Cache* c = _caches[co_id];
      Cache::Stat s = c->GetStat();
      Cons::P(boost::format(fmt) % co_id % s.hits % s.misses % s.num_items);
    }
  }

  void _ReportStat(long total_cache_size) {
    long hits = 0;
    long misses = 0;
    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;
      Cache* c = _caches[co_id];
      Cache::Stat s = c->GetStat();
      //Cons::P(boost::format(fmt) % co_id % s.hits % s.misses % s.num_items);
      hits += s.hits;
      misses += s.misses;
    }

    Cons::P(boost::format("%d %f %d %d") % total_cache_size % (double(hits) / (hits + misses)) % hits % misses);
  }
}
