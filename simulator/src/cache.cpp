#include "cache.h"
#include "cons.h"
#include "util.h"


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


void Cache::Resize(long c) {
  _capacity = c;

  if (0 < _l.size())
    THROW("Implement when needed");
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
