#pragma once

// An LRU cache implementation

#include <list>
#include <map>

class Cache {
  // List of cache items. The newest one is at the front; the oldest is at the back. 
  //   CacheItem*. For now, just use video_id (string).
	std::list<std::string> _l;
	std::map<std::string, std::list<std::string>::iterator > _c;

	long _capacity = 0;
  long _occupancy = 0;

	// Cache hits and misses
	long _hits = 0;
	long _misses = 0;

public:
  Cache(long c);

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


namespace Caches {
  void Init();
  void PlayWorkload();
  void FreeMem();
  void ShowStat();
};
