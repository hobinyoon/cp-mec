// World includes all dynamic components for replaying a workload on a network infrastructure

#pragma once

#include "data-access.h"

class World {
public:
  World(long aggr_cache_size);

  void PlayWorkload(const char* fmt);

private:
  long _aggr_cache_size;
  std::vector<double> _latencies;

  void _InitOriginDCs();
  void _AllocCaches();
  void _ReportStat(const char* fmt);
  void _AllocCachesUniform();
  void _AllocCachesReqVolBased();
  void _AllocCachesUserBased();
  void _AllocCachesUtilityBased();

  struct PwParam;
  void _PlayWorkload(PwParam* p);
};
