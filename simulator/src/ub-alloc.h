// Utility-based cache space allocator

#pragma once

#include <map>

namespace UbAlloc {
  void Calc(const long total_cache_size,  std::map<int, long>& coid_cachesize);
};
