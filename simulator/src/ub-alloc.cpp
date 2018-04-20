#include "cons.h"
#include "ub-alloc.h"
#include "util.h"
#include "utility-curve.h"
#include "youtube-access.h"

using namespace std;

namespace UbAlloc {
  struct EL {
    // Current cache size and bytes served from the cache
    long _cur_size;
    double _cur_gain;

    // Utility curve
    const map<long, long>* _uc;
    map<long, long>::const_iterator _uc_it;

    EL(const map<long, long>* uc)
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
      double dy = next_gain - _cur_gain;
      double next_gain_delta = 50 * dy / dx;

      _cur_size += 50;
      _cur_gain += next_gain_delta;

      // When _cur_size passes next_size, move _uc_it by 1
      //   Think about what to do when _cur_size doesn't increase by 50, when needed.
      if (_cur_size == next_size) {
        // This removes the floating point arithmetic error.
        _cur_gain = next_gain;
        _uc_it ++;
      }

      //Cons::P(boost::format("                           %d %f %f %d %d") % _cur_size % _cur_gain % next_gain_delta % dx % dy);

      return next_gain_delta;
    }
  };


  // Calculate cache size at each EL using the utility curves
  //   elid_cachesize: map<el_id, cache_size_to_allocate_in_MB>
  void Calc(const long total_cache_size,  map<int, long>& elid_cachesize) {
    // map<el_id, EL*>
    map<int, EL*> elid_el;
    for (const auto& i: UtilityCurves::Get()) {
      int el_id = i.first;
      const auto uc = i.second;
      elid_el.emplace(el_id, new EL(uc));
      elid_cachesize.emplace(el_id, 0);
    }

    // Initialize map<next_gain_delta, vector<el_id> >
    map<double, vector<int> > ngd_elids;
    for (auto i: elid_el) {
      int el_id = i.first;
      double ngd = i.second->GetNextGainDelta();
      //Cons::P(boost::format("Init %d %f") % el_id % ngd);
      if (ngd == 0.0)
        continue;
      auto it = ngd_elids.find(ngd);
      if (it == ngd_elids.end())
        ngd_elids.emplace(ngd, vector<int>());
      ngd_elids[ngd].push_back(el_id);
    }

    // Keep allocating 50 MB of cache space until the total space becomes total_cache_size
    for (long s = 0; s < total_cache_size; s += 50) {
      if (ngd_elids.size() == 0) {
        Cons::P(boost::format("Can't allocate %d MB-th space. total_cache_size=%d") % s % total_cache_size);
        break;
      }

      auto it = ngd_elids.rbegin();
      //double gain_delta = it->first;
      auto& el_ids = it->second;
      if (el_ids.size() == 0)
        THROW("Unexpected");

      int el_id = el_ids.back();
      //Cons::P(boost::format("Alloc %d") % el_id);

      auto it1 = elid_cachesize.find(el_id);
      if (it1 == elid_cachesize.end())
        THROW("Unexpected");
      it1->second += 50;

      // Erase the el_id. The amortized cost of erasing at the end is O(1)
      el_ids.pop_back();
      if (el_ids.size() == 0) {
        // Reverse iterator and base() are off by 1
        //   http://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        ngd_elids.erase((++it).base());
      }

      // Update ngd_elids with the next ngd of the just-deleted EL
      EL* el = elid_el[el_id];
      double ngd = el->GetNextGainDelta();
      //Cons::P(boost::format("Alloc %d %d %f") % el_id % it1->second % ngd);
      if (ngd != 0.0) {
        auto it2 = ngd_elids.find(ngd);
        if (it2 == ngd_elids.end())
          ngd_elids.emplace(ngd, vector<int>());
        ngd_elids[ngd].push_back(el_id);
      }
    }

    // Free memory
    for (auto i: elid_el)
      delete i.second;
    elid_el.clear();
  }
};
