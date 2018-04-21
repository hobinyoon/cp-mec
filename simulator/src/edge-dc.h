#pragma once

#include "cache.h"

template <class T>
class EdgeDC {
  Cache<T> _cache;

  long _traffic_o2c = 0;
  long _traffic_c2u = 0;

public:
  EdgeDC(long cache_size)
    : _cache(cache_size)
  {}

  bool Get(const T& item_key) {
    return _cache.Get(item_key);
  }

  // Returns true when the cache item was put in the cache. False otherwise.
  bool Put(const T& item_key, long item_size) {
    bool r = _cache.Put(item_key, item_size);
    return r;
  }

  void FetchFromOrigin(long item_size) {
    _traffic_o2c += item_size;
  }

  void ServeDataToUser(long item_size) {
    _traffic_c2u += item_size;
  }

  struct Stat {
    typename Cache<T>::Stat cache_stat;
    long traffic_o2c;
    long traffic_c2u;

    Stat(const typename Cache<T>::Stat cache_stat_, long traffic_o2c_, long traffic_c2u_)
      : cache_stat(cache_stat_), traffic_o2c(traffic_o2c_), traffic_c2u(traffic_c2u_)
    {}
  };

  Stat GetStat() {
    return Stat(_cache.GetStat(), _traffic_o2c, _traffic_c2u);
  }
};
