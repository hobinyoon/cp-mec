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

  void Init() {
    Cons::MT _("Initializing caches ...");

    // TODO: allocate some amount of cache uniformly on every node
    //   Start by looking at the amount of cache space for perfect caching. Get it from UtilityCurves

    for (const auto i: YoutubeAccess::CoAccesses()) {
      int co_id = i.first;

      // 50 MB of cache capacity everywhere as a start
      Cache* c = new Cache(3 * 50);
      _caches.emplace(co_id, c);
    }
  }

  void PlayWorkload() {
    Cons::MT _("Playing workload ...");

    // TODO: this should be parallelized

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

  void ShowStat() {
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

    Cons::P(boost::format("%f %d %d") % (double(hits) / (hits + misses)) % hits % misses);
  }
}
