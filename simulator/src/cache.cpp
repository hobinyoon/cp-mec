#include "cache.h"
#include "cons.h"
#include "util.h"
#include "youtube-access.h"

using namespace std;


Cache::Cache(long c)
  : _capacity(c)
{
}


Cache::~Cache()
{
  _c.clear();

  for (auto i: _l)
    delete i;
  _l.clear();
}


bool Cache::Get(const string& item_key) {
  auto it = _c.find(item_key);
  if (it == _c.end()) {
    _misses ++;
    return false;
  }

  // Move the cache item to the front of the list
  Item* item = *(it->second);
  _l.erase(it->second);
  auto it1 = _l.insert(_l.begin(), item);

  // Update _c to point the cache item location in _l
  //   Use []. emplace() doesn't guarantee replacing the existing element.
  //     Wait... really? A reference?
  //auto p = _c.emplace(op_read->obj_id, it1);
  //Cons::P(boost::format("emplace2: %s") % p.second);
  _c[item_key] = it1;
  _hits ++;
  return true;
}


// Returns true when the cache item was put in the cache. False otherwise.
bool Cache::Put(const string& item_key, long item_size) {
  // Make sure the cache item wasn't there before. We don't allow duplicate items in the cache with the same key.
  if (_c.find(item_key) != _c.end())
    THROW("Unexpected");

  // When out of space, evict the oldest item (at the back)
  while (_capacity < _occupancy + item_size) {
    if (_l.size() == 0) {
      // Evicted all items, but still can't cache the new item.
      return false;
    }

    auto it = _l.rbegin();
    Item* item = *it;

    _c.erase(item->key);

    // Delete the pointer to the cache item in the list
    //   Reverse iterator and base() are off by 1
    //     http://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
    _l.erase((++it).base());

    _occupancy -= item->size;

    // Delete the cache item
    delete item;
  }

  Item* item = new Item(item_key, item_size);

  // Insert to the front
  auto it = _l.insert(_l.begin(), item);
  _c[item_key] = it;
  _occupancy += item_size;
  return true;
}


Cache::Stat::Stat(long hits_, long misses_, long num_items_)
  : hits(hits_), misses(misses_), num_items(num_items_)
{
}

Cache::Stat Cache::GetStat() {
  return Stat(_hits, _misses, _c.size());
}


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
