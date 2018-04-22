#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include "conf.h"
#include "cons.h"
#include "data-access.h"
#include "data-source.h"
#include "edge-dc.h"
#include "util.h"

using namespace std;


EdgeDC::EdgeDC(const vector<string>& t) {
  _id = stoi(t[0]);
  _lat = stod(t[1]);
  _lon = stod(t[2]);
}


EdgeDC::EdgeDC(std::ifstream& ifs) {
  ifs.read((char*)&_id, sizeof(_id));
  ifs.read((char*)&_lat, sizeof(_lat));
  ifs.read((char*)&_lon, sizeof(_lon));
}


void EdgeDC::WriteCondensed(ofstream& ofs) {
  ofs.write((char*)&_id, sizeof(_id));
  ofs.write((char*)&_lat, sizeof(_lat));
  ofs.write((char*)&_lon, sizeof(_lon));
}


int EdgeDC::Id() {
  return _id;
}


double EdgeDC::Lat() {
  return _lat;
}


double EdgeDC::Lon() {
  return _lon;
}


void EdgeDC::DeallocCache() {
  _cache.Dealloc();
  _traffic_o2c = 0;
  _traffic_c2u = 0;
}


void EdgeDC::AllocCache(long cache_size) {
  _cache.SetCapacity(cache_size);
}


bool EdgeDC::GetObj(const int& item_key) {
  return _cache.Get(item_key);
}


// Returns true when the cache item was put in the cache. False otherwise.
bool EdgeDC::PutObj(const int& item_key, long item_size) {
  bool r = _cache.Put(item_key, item_size);
  return r;
}


void EdgeDC::FetchFromOrigin(long item_size) {
  _traffic_o2c += item_size;
}


void EdgeDC::ServeDataToUser(long item_size) {
  _traffic_c2u += item_size;
}


double EdgeDC::LatencyToOrigin() {
  // TODO: implement
  return 0.0;
}


EdgeDC::Stat EdgeDC::GetStat() {
  return Stat(_cache.GetStat(), _traffic_o2c, _traffic_c2u);
}


namespace EdgeDCs {
  namespace bf = boost::filesystem;

  map<int, EdgeDC*> _id_edc;
  // Edge DC to a nearest data source node (core data center)
  map<int, DataSource::Node*> _edcid_dc;


  EdgeDC* Add(const vector<string>& t) {
    EdgeDC* edc = new EdgeDC(t);
    _id_edc.emplace(edc->Id(), edc);
    return edc;
  }


  void Delete(int id) {
    auto it = _id_edc.find(id);
    if (it == _id_edc.end())
      THROW(boost::format("Unexpected [%d]") % id);
    delete it->second;
    _id_edc.erase(it);
  }


  size_t NumDCs() {
    return _id_edc.size();
  }


  const map<int, EdgeDC*>& Get() {
    return _id_edc;
  }


  EdgeDC* Get(int edc_id) {
    auto it = _id_edc.find(edc_id);
    if (it == _id_edc.end())
      THROW(boost::format("Unexpected [%d]") % edc_id);
    return it->second;
  }


  void WriteCondensed(const string& fn) {
    Cons::MT _(boost::format("Generating condensed file %s") % fn);
    ofstream ofs(fn);

    size_t num_edcs = _id_edc.size();
    ofs.write((char*)&num_edcs, sizeof(num_edcs));

    for (auto i: _id_edc) {
      int edc_id = i.first;
      i.second->WriteCondensed(ofs);

      AllDAs::WriteCondensed(edc_id, ofs);
    }
    ofs.close();

    Cons::P(boost::format("Created %s %d") % fn % bf::file_size(fn));
  }


  void LoadCondensed(const string& fn) {
    Cons::MT _(boost::format("Loading condensed YouTube video accesses by edge DCs from %s ...") % fn);

    int max_el_id = stoi(Conf::Get("max_el_id"));
    ifstream ifs(fn);

    size_t num_edcs;
    ifs.read((char*)&num_edcs, sizeof(num_edcs));

    for (size_t i = 0; i < num_edcs; i ++) {
      EdgeDC* edc = new EdgeDC(ifs);

      int edc_id = edc->Id();
      if (max_el_id != -1 && max_el_id < edc_id) {
        Cons::P(boost::format("Passed max_el_id %d. Stop loading") % max_el_id);
        delete edc;
        break;
      }
      _id_edc.emplace(edc_id, edc);

      AllDAs::LoadCondensed(edc_id, ifs);
    }
    Cons::P(boost::format("Loaded video access reqs from %d edge DCs.") % _id_edc.size());
  }


  void MapEdcToDatasource() {
    for (auto i: _id_edc) {
      int edc_id = i.first;
      EdgeDC* edc = i.second;

      DataSource::Node* n = DataSource::GetNearest(edc->Lat(), edc->Lon());
      _edcid_dc.emplace(edc_id, n);
    }
  }


  void DeallocCaches() {
    for (auto i: _id_edc) {
      //int edc_id = i.first;
      EdgeDC* edc = i.second;
      edc->DeallocCache();
    }
  }


  void FreeMem() {
    for (auto i: _id_edc)
      delete i.second;
    _id_edc.clear();
  }
};
