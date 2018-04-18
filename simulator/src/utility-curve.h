#pragma once

#include <map>

namespace UtilityCurves {
  void Load();
  void FreeMem();
  long SumMaxLruCacheSize();

  const std::map<int, std::map<long, long>* >& Get();
};
