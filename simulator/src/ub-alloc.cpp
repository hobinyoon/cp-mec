#include "cons.h"
#include "ub-alloc.h"
#include "util.h"
#include "utility-curve.h"
#include "youtube-access.h"

using namespace std;

namespace UbAlloc {
  struct CO {
    // Current cache size and bytes served from the cache
    long _cur_size;
    double _cur_gain;

    // Utility curve
    const map<long, long>* _uc;
    map<long, long>::const_iterator _uc_it;

    CO(const map<long, long>* uc)
      : _cur_size(0), _cur_gain(0.0), _uc(uc)
    {
      if (_uc->size() == 0)
        return;

      _uc_it = _uc->begin();
    }

    // The gain for the next 50 MB of cache space
    //   This will be the map<> key outside of this struct.
    double GetNextGainDelta() {
      if (_uc->size() == 0)
        return 0.0;
      if (_uc_it == _uc->end())
        return 0.0;

      long next_size = _uc_it->first;
      long next_gain = _uc_it->second;

      if (next_gain <= _cur_gain)
        THROW("Unexpected");
      if (next_size <= _cur_size)
        THROW("Unexpected");

      long dx = next_size - _cur_size;
      long dy = next_gain - _cur_gain;
      double next_gain_delta = 50 * double(dy) / dx;

      _cur_size += 50;
      _cur_gain += next_gain_delta;

      // This removes the floating point arithmetic error.
      //   Think about what to do when _cur_size doesn't increase by 50, when needed.
      if (_cur_size == next_size) {
        _cur_gain = next_gain;
        _uc_it ++;
      }

      return next_gain_delta;
    }
  };


  // Calculate cache size at each CO using the utility curves
  //   coid_cachesize: map<co_id, cache_size_to_allocate_in_MB>
  void Calc(const long total_cache_size,  map<int, long>& coid_cachesize) {
    // map<co_id, CO*>
    map<int, CO*> coid_co;
    for (const auto& i: UtilityCurves::Get()) {
      int co_id = i.first;
      const auto uc = i.second;
      coid_co.emplace(co_id, new CO(uc));
      coid_cachesize.emplace(co_id, 0);
    }

    //map<next_gain_delta, vector<co_id> >
    map<double, vector<int> > ngd_coids;
    for (auto i: coid_co) {
      int co_id = i.first;
      double ngd = i.second->GetNextGainDelta();
      if (ngd == 0.0)
        continue;
      auto it = ngd_coids.find(ngd);
      if (it == ngd_coids.end())
        ngd_coids.emplace(ngd, vector<int>());
      ngd_coids[ngd].push_back(co_id);
    }

    // Keep allocating 50 MB of cache space until the total space becomes total_cache_size
    for (long s = 0; s < total_cache_size; s += 50) {
      if (ngd_coids.size() == 0) {
        Cons::P(boost::format("Can't allocate %d MB-th space. total_cache_size=%d") % s % total_cache_size);
        break;
      }

      auto it = ngd_coids.rbegin();
      //double gain_delta = it->first;
      auto& co_ids = it->second;
      if (co_ids.size() == 0)
        THROW("Unexpected");

      int co_id = co_ids.back();

      auto it1 = coid_cachesize.find(co_id);
      if (it1 == coid_cachesize.end())
        THROW("Unexpected");
      it1->second += 50;

      // Erase the co_id. The amortized cost of erasing at the end is O(1)
      co_ids.pop_back();
      if (co_ids.size() == 0) {
        // Reverse iterator and base() are off by 1
        //   http://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        ngd_coids.erase((++it).base());
      }

      // Update ngd_coids with the next ngd of the just-deleted CO
      CO* co = coid_co[co_id];
      double ngd = co->GetNextGainDelta();
      if (ngd != 0.0) {
        auto it2 = ngd_coids.find(ngd);
        if (it2 == ngd_coids.end())
          ngd_coids.emplace(ngd, vector<int>());
        ngd_coids[ngd].push_back(co_id);
      }
    }

    // Free memory
    for (auto i: coid_co)
      delete i.second;
    coid_co.clear();
  }
};