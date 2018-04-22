#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "cache.h"
#include "data-source.h"

class EdgeDC {
  // These don't change over experiments (World runs).
  int _id;
  double _lat;
  double _lon;
  double _lat_to_datasource = 0;

  // Mutable by each experiment
  Cache<int> _cache;
  long _traffic_o2c = 0;
  long _traffic_c2u = 0;

public:
  EdgeDC(const std::vector<std::string>& t);
  EdgeDC(std::ifstream& ifs);
  void WriteCondensed(std::ofstream& ofs);

  int Id();
  double Lat();
  double Lon();

  void SetDatasource(DataSource::Node* ds);
  void DeallocCache();
  void AllocCache(long cache_size);

  bool GetObj(const int& item_key);
  // Returns true when the cache item was put in the cache. False otherwise.
  bool PutObj(const int& item_key, long item_size);
  void FetchFromOrigin(long item_size);
  void ServeDataToUser(long item_size);
  double LatencyToDatasource();

  struct Stat {
    typename Cache<int>::Stat cache_stat;
    long traffic_o2c;
    long traffic_c2u;

    Stat(const typename Cache<int>::Stat cache_stat_, long traffic_o2c_, long traffic_c2u_)
      : cache_stat(cache_stat_), traffic_o2c(traffic_o2c_), traffic_c2u(traffic_c2u_)
    {}
  };

  Stat GetStat();
};


namespace EdgeDCs {
  EdgeDC* Add(const std::vector<std::string>& t);
  void Delete(int id);
  const std::map<int, EdgeDC*>& Get();
  EdgeDC* Get(int edc_id);
  size_t NumDCs();
  void WriteCondensed(const std::string& fn);
  void LoadCondensed(const std::string& fn);
  void MapEdcToDatasource();
  void DeallocCaches();
  void FreeMem();
};
