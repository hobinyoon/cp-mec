#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "data-access.h"
#include "util.h"

using namespace std;


void DAs::AddSingleAccess(const string& line) {
  if (line.size() < 2)
    THROW(boost::format("Unexpected [%s]") % line);
  string line2 = line.substr(2);
  vector<string> t;
  static const auto sep = boost::is_any_of(" ");
  boost::split(t, line2, sep, boost::token_compress_on);
  // tweet_id user_id created_at latitude longitude youtube_video_id dist_to_CO
  if (t.size() != 8)
    THROW(boost::format("Unexpected %d [%s]") % t.size() % line2);

  _user_ids.insert(t[1]);
  _obj_ids_str.push_back(t[6]);
}


void DAs::ConvStringObjToIntObj() {
  map<string, int> objid_int;
  int idx = 0;
  for (auto& obj_id: _obj_ids_str) {
    auto it = objid_int.find(obj_id);
    if (it == objid_int.end()) {
      objid_int.emplace(obj_id, idx);
      idx ++;
    }
  }

  for (auto& obj_id: _obj_ids_str) {
    auto it = objid_int.find(obj_id);
    if (it == objid_int.end())
      THROW("Unexpected");
    _obj_ids.push_back(it->second);
  }
}


void DAs::WriteCondensed(ofstream& ofs) {
  {
    size_t n = _user_ids.size();
    ofs.write((char*)&n, sizeof(n));
  }
  {
    size_t n = _obj_ids.size();
    ofs.write((char*)&n, sizeof(n));
  }

  for (int obj_id: _obj_ids)
    ofs.write((char*)&obj_id, sizeof(obj_id));
}


void DAs::LoadCondensed(ifstream& ifs) {
  ifs.read((char*)&_num_users, sizeof(_num_users));

  size_t num_obj_ids;
  ifs.read((char*)&num_obj_ids, sizeof(num_obj_ids));

  for (size_t i = 0; i < num_obj_ids; i ++) {
    int obj_id;
    ifs.read((char*)&obj_id, sizeof(obj_id));
    _obj_ids.push_back(obj_id);
  }
}


size_t DAs::NumAccesses() {
  return _obj_ids.size();
}

int DAs::NumUsers() {
  return max(_user_ids.size(), _num_users);
}

const std::vector<int>& DAs::ObjIds() const {
  return _obj_ids;
}


namespace AllDAs {
  map<int, DAs*> _edc_accesses;


  void AddSingleAccess(const int edc_id, const std::string& line) {
    auto it = _edc_accesses.find(edc_id);
    DAs* acc;
    if (it == _edc_accesses.end()) {
      acc = new DAs();
      _edc_accesses.emplace(edc_id, acc);
    } else {
      acc = it->second;
    }
    acc->AddSingleAccess(line);
  }


  void ConvStringObjToIntObj() {
    for (auto i: _edc_accesses)
      i.second->ConvStringObjToIntObj();
  }


  void FreeMem() {
    for (auto i: _edc_accesses)
      delete i.second;
    _edc_accesses.clear();
  }


  const map<int, DAs* >& Get() {
    return _edc_accesses;
  }


  const DAs* Get(int edc_id) {
    auto it = _edc_accesses.find(edc_id);
    if (it == _edc_accesses.end())
      THROW(boost::format("Unexpected [%d]") % edc_id);
    return it->second;
  }


  void WriteCondensed(int edc_id, ofstream& ofs) {
    auto it = _edc_accesses.find(edc_id);
    if (it == _edc_accesses.end())
      THROW(boost::format("Unexpected [%d]") % edc_id);

    DAs* acc = it->second;
    acc->WriteCondensed(ofs);
  }


  void LoadCondensed(int edc_id, ifstream& ifs) {
    auto it = _edc_accesses.find(edc_id);
    if (it != _edc_accesses.end())
      THROW(boost::format("Unexpected [%d]") % edc_id);

    DAs* acc = new DAs();
    acc->LoadCondensed(ifs);

    _edc_accesses.emplace(edc_id, acc);
  }
};
