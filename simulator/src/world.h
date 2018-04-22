#pragma once

#include "edge-dc.h"
#include "youtube-access.h"

class World {
public:
  World(long aggr_cache_size);
  ~World();

  void PlayWorkload();
  void ReportStat(const char* fmt);

private:
  long _aggr_cache_size;
  //map<EL(edge_location)_id, EdgeDC* >
  std::map<int, EdgeDC* > _edgedcs;

  void _InitOriginDCs();
  void _AllocateCaches();
  void _ReportStat();
  void _AllocateCachesUniform();
  void _AllocateCachesReqVolBased();
  void _AllocateCachesUserBased();
  void _AllocateCachesUtilityBased();
  void _PlayWorkload(
      std::map<int, YoutubeAccess::Accesses* >::const_iterator it_begin,
      std::map<int, YoutubeAccess::Accesses* >::const_iterator it_end);
};
