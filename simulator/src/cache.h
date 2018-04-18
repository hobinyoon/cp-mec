#pragma once

#include <list>
#include <map>

// An LRU cache implementation

class Cache {
  // Cache item
  struct Item {
    std::string key;
    long size;
    Item(std::string key_, long size_)
      : key(key_), size(size_)
    {}
  };

  // List of cache items. The newest one is at the front; the oldest is at the back. 
  std::list<Item*> _l;
  std::map<std::string, std::list<Item*>::iterator > _c;

  long _capacity = 0;
  long _occupancy = 0;

  // Cache hits and misses
  long _hits = 0;
  long _misses = 0;

public:
  Cache(long c);
  ~Cache();

  void Resize(long c);

  // Returns true when the video with key exists. False otherwise.
  bool Get(const std::string& item_key);

  bool Put(const std::string& k, long item_size);

  struct Stat {
    long hits;
    long misses;
    long num_items;

    Stat(long hits, long misses, long num_items);
  };

  Stat GetStat();
};
