#include <map>
#include <thread>
#include <vector>

#include "cache.h"
#include "conf.h"
#include "cons.h"
#include "data-access.h"
#include "edge-dc.h"
#include "ub-alloc.h"
#include "util.h"
#include "utility-curve.h"
#include "world.h"


World::World(long aggr_cache_size) {
  _aggr_cache_size = aggr_cache_size;
  _AllocCaches();
}


void World::PlayWorkload(const char* fmt) {
  //Cons::MT _("Playing workload ...");

  int num_threads = Util::NumHwThreads();
  size_t n = AllDAs::Get().size();

  // First few threads may have 1 more work items than the other threads due to the integer devision rounding.
  long s1 = n / num_threads;
  long s0 = s1 + 1;

  // Allocate s0 work items for the first a threads and s1 items for the next b threads
  //   a * s0 + b * s1 = n
  //   a + b = num_threads
  //
  //   a * s0 + (num_threads - a) * s1 = n
  //   a * (s0 - s1) + num_threads * s1 = n
  //   a = (n - num_threads * s1) / (s0 - s1)
  //   a = n - num_threads * s1
  int a = n - num_threads * s1;
  //Cons::P(boost::format("n=%d num_threads=%d s0=%d s1=%d a=%d") % n % num_threads % s0 % s1 % a);

  std::vector<std::thread> threads;

  // const std::map<int, DAs* >& Get();
  auto it_begin = AllDAs::Get().begin();

  int i = 0;
  for (; i < a; i ++) {
    auto it_end = it_begin;
    std::advance(it_end, s0);
    threads.push_back(std::thread(&World::_PlayWorkload, this, it_begin, it_end));
    it_begin = it_end;
  }
  for (; i < num_threads; i ++) {
    auto it_end = it_begin;
    std::advance(it_end, s1);
    threads.push_back(std::thread(&World::_PlayWorkload, this, it_begin, it_end));
    it_begin = it_end;
  }
  //Cons::P(boost::format("Running %d threads") % threads.size());
  for (auto& t: threads)
    t.join();

  // You don't need to gather the results here. Each thread updates parts of the EdgeDC instances.

  _ReportStat(fmt);
}


void World::_AllocCaches() {
  //Cons::MT _("Allocating caches ...");
  EdgeDCs::DeallocCaches();

  std::string p = Conf::Get("placement_stragegy");
  if (p == "uniform") {
    _AllocCachesUniform();
  } else if (p == "req_volume_based") {
    _AllocCachesReqVolBased();
  } else if (p == "user_based") {
    _AllocCachesUserBased();
  } else if (p == "utility_based") {
    _AllocCachesUtilityBased();
  } else {
    THROW("Unexpected");
  }
}


void World::_ReportStat(const char* fmt) {
  long hits = 0;
  long misses = 0;
  long o2c = 0;
  long c2u = 0;
  for (const auto i: EdgeDCs::Get()) {
    //int edc_id = i.first;
    EdgeDC* edc = i.second;
    EdgeDC::Stat s = edc->GetStat();
    //Cons::P(boost::format(fmt) % edc_id % s.hits % s.misses % s.num_items);
    hits += s.cache_stat.hits;
    misses += s.cache_stat.misses;
    o2c += s.traffic_o2c;
    c2u += s.traffic_c2u;
  }

  // 25353550 0.081899 124079 1390937 69546850 75750800
  Cons::P(boost::format(fmt) % _aggr_cache_size % (double(hits) / (hits + misses)) % hits % misses % o2c % c2u);
}


void World::_AllocCachesUniform() {
  size_t num_edcs = EdgeDCs::Get().size();

  // Some caches have 1 bytes bigger size than the others due to the integer division.
  long cache_size_0 = _aggr_cache_size / num_edcs;
  long cache_size_1 = cache_size_0 + 1;

  // Allocate cache_size_0 MB for the first a caches and cache_size_1 MB for the others
  // a * cache_size_0 + b * cache_size_1 = _aggr_cache_size
  // a + b = num_edcs
  //
  // a * cache_size_0 + (num_edcs - a) * cache_size_1 = _aggr_cache_size
  // a * (cache_size_0 - cache_size_1) + num_edcs * cache_size_1 = _aggr_cache_size
  // a = (_aggr_cache_size - num_edcs * cache_size_1) / (cache_size_0 - cache_size_1)
  // a = num_edcs * cache_size_1 - _aggr_cache_size
  int a = num_edcs * cache_size_1 - _aggr_cache_size;

  long cache_size = cache_size_0;
  int j = 0;
  for (const auto i: EdgeDCs::Get()) {
    j ++;
    if (a < j)
      cache_size = cache_size_1;

    //Cons::P(boost::format("%d") % cache_size);
    //int edc_id = i.first;
    EdgeDC* edc = i.second;
    edc->AllocCache(cache_size);
  }
}


void World::_AllocCachesReqVolBased() {
  long total_num_reqs = 0;
  for (auto i: AllDAs::Get())
    total_num_reqs += i.second->NumAccesses();

  std::map<int, long> edcid_cachesize;
  long total_allocated = 0;
  for (auto i: AllDAs::Get()) {
    int edc_id = i.first;
    size_t num_reqs = i.second->NumAccesses();
    long s = double(_aggr_cache_size) * num_reqs / total_num_reqs;
    edcid_cachesize.emplace(edc_id, s);
    total_allocated += s;
  }

  long remainder = _aggr_cache_size - total_allocated;
  //Cons::P(boost::format("_aggr_cache_size=%d total_allocated=%d remainder=%d")
  //    % _aggr_cache_size % total_allocated % remainder);

  //long alloc0 = 0;
  //for (auto& i: edcid_cachesize)
  //  alloc0 += i.second;

  long j = 0;
  for (auto& i: edcid_cachesize) {
    i.second ++;
    j ++;
    if (j == remainder)
      break;
  }

  //long alloc1 = 0;
  //for (auto& i: edcid_cachesize)
  //  alloc1 += i.second;
  //Cons::P(boost::format("alloc0=%d alloc1=%d") % alloc0 % alloc1);

  for (const auto i: EdgeDCs::Get()) {
    int edc_id = i.first;
    EdgeDC* edc = i.second;

    auto it = edcid_cachesize.find(edc_id);
    if (it == edcid_cachesize.end())
      THROW(boost::format("Unexpected. edc_id=%d") % edc_id);
    edc->AllocCache(it->second);
  }
}


void World::_AllocCachesUserBased() {
  long total_num_users = 0;
  for (auto i: AllDAs::Get())
    total_num_users += i.second->NumUsers();

  std::map<int, long> edcid_cachesize;
  long total_allocated = 0;
  for (auto i: AllDAs::Get()) {
    int edc_id = i.first;
    size_t num_users = i.second->NumUsers();
    long s = double(_aggr_cache_size) * num_users / total_num_users;
    edcid_cachesize.emplace(edc_id, s);
    total_allocated += s;
  }

  long remainder = _aggr_cache_size - total_allocated;
  //Cons::P(boost::format("_aggr_cache_size=%d total_allocated=%d remainder=%d")
  //    % _aggr_cache_size % total_allocated % remainder);

  //long alloc0 = 0;
  //for (auto& i: edcid_cachesize)
  //  alloc0 += i.second;

  long j = 0;
  for (auto& i: edcid_cachesize) {
    i.second ++;
    j ++;
    if (j == remainder)
      break;
  }

  //long alloc1 = 0;
  //for (auto& i: edcid_cachesize)
  //  alloc1 += i.second;
  //Cons::P(boost::format("alloc0=%d alloc1=%d") % alloc0 % alloc1);

  for (const auto i: EdgeDCs::Get()) {
    int edc_id = i.first;
    EdgeDC* edc = i.second;

    auto it = edcid_cachesize.find(edc_id);
    if (it == edcid_cachesize.end())
      THROW(boost::format("Unexpected. edc_id=%d") % edc_id);
    edc->AllocCache(it->second);
  }
}


void World::_AllocCachesUtilityBased() {
  //Cons::MT _("Allocate cache space using utility curves ...");

  std::map<int, long> edcid_cachesize;
  UbAlloc::Calc(_aggr_cache_size, edcid_cachesize);

  for (const auto i: EdgeDCs::Get()) {
    int edc_id = i.first;
    EdgeDC* edc = i.second;

    auto it = edcid_cachesize.find(edc_id);
    if (it == edcid_cachesize.end())
      THROW(boost::format("Unexpected. edc_id=%d") % edc_id);
    edc->AllocCache(it->second);
  }
}


void World::_PlayWorkload(
    std::map<int, DAs* >::const_iterator it_begin,
    std::map<int, DAs* >::const_iterator it_end) {
  // Data access latency calculation
  //   Wireless
  //     Latency between user to the closest BS: follow the 4G model.
  //       We don't know the dist between a user and the closest BS from it, but we don't need it.
  //     BS to CO: use the average calculated latency in the Castnet paper draft.
  //     CO to origin: use the distance.
  //   Wired
  //     User to Edge: use the distance
  //     Edge to origin: use the distance
  static const double lat_4g = Conf::GetNode("lat_4g").as<double>();
  static const double lat_bs_to_co = Conf::GetNode("lat_bs_to_co").as<double>();

  for (auto it = it_begin; it != it_end; it ++) {
    int edc_id = it->first;
    EdgeDC* edc = EdgeDCs::Get(edc_id);

    const DAs* acc = AllDAs::Get(edc_id);
    for (const auto& item_key: acc->ObjIds()) {
      // Item size is 50 MB for every video: an wild guess of the average YouTube video size downloaded.
      const long item_size = 50;
      double lat = lat_4g + lat_bs_to_co;
      // TODO: keep going

      if (edc->GetObj(item_key)) {
        // The item is served (downloaded) from the cache
      } else {
        // The item is not in the cache.

        // Fetch the data item from the origin to the edge cache
        edc->FetchFromOrigin(item_size);
        lat += edc->LatencyToOrigin();

        // Put the item in the cache.
        //   The operation can fail when there isn't enough space for the new item even after evicting existing items.
        //     For example when the cache size is 0.
        //   We don't do anything about the failed operation for now.
        edc->PutObj(item_key, item_size);
      }
      edc->ServeDataToUser(item_size);
    }
  }
}


//void _ReportStatPerCache() {
//  std::string fmt = "%3d %3d %4d %3d";
//  std::string header = Util::BuildHeader(fmt, "edc_id cache_hits cache_misses num_items_in_cache");
//  Cons::P(header);
//  for (const auto i: EdcDaLoader::EdcAccesses()) {
//    int edc_id = i.first;
//    EdgeDC* edc = _edgedcs[edc_id];
//    EdgeDC::Stat s = edc->GetStat();
//    Cons::P(boost::format(fmt) % edc_id % s.hits % s.misses % s.num_items);
//  }
//}
