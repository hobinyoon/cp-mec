#include <fstream>
#include <mutex>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "central-office.h"
#include "conf.h"
#include "cons.h"

#include "util.h"
#include "youtube-access.h"

using namespace std;

namespace YoutubeAccess {
  namespace bf = boost::filesystem;

  // map<co_id, vector<video_id_accessed> >
  map<int, vector<int>* > _coid_accesses;

  void _Load(const string& fn);
  void _WriteCondensed(const string& fn);
  bool _LoadCondensed(const string& fn);
  void _ConvStrId2IntId(const vector<string>& obj_ids_str, vector<int>* obj_ids_int);


  void Load() {
    string fn = Conf::GetFn("video_accesses_by_COs");
    if (! bf::exists(fn))
      THROW("Unexpected");

    if (_LoadCondensed(fn))
      return;
    _Load(fn);
    _WriteCondensed(fn);
  }


  void _Load(const string& fn) {
    Cons::MT _(boost::format("Loading YouTube video accesses by COs from %s ...") % fn);

    int max_co_id = atoi(Conf::GetStr("max_co_id").c_str());

    ifstream ifs(fn);
    string line;
    while (getline(ifs, line)) {
      if ((line.size() == 0) || (line[0] == '#'))
        continue;
      //Cons::P(line);

      // central_office_id latitude longitude num_video_accesses
      static const auto sep = boost::is_any_of(" ");
      vector<string> t;
      boost::split(t, line, sep);
      if (t.size() != 4)
        THROW(boost::format("Unexpected [%s]") % line);

      int co_id = atoi(t[0].c_str());
      if (max_co_id != -1 && max_co_id < co_id) {
        Cons::P(boost::format("Passed max_co_id %d. Stop loading") % max_co_id);
        break;
      }

      int num_accesses = atoi(t[3].c_str());
      if (num_accesses <= 1) {
        THROW(boost::format("Unexpected [%s]") % line);
        continue;
      }

      vector<string> obj_ids_str;
      for (int i = 0; i < num_accesses; i ++) {
        if (! getline(ifs, line))
          THROW(boost::format("Unexpected [%s]") % line);
        if (line.size() < 2)
          THROW(boost::format("Unexpected [%s]") % line);
        string line2 = line.substr(2);
        vector<string> t;
        boost::split(t, line2, sep, boost::token_compress_on);
        // tweet_id user_id created_at(date_time) latitude longitude youtube_video_id
        if (t.size() != 7)
          THROW(boost::format("Unexpected %d [%s]") % t.size() % line2);
        obj_ids_str.push_back(t[6]);
      }

      vector<int>* obj_ids_int = new vector<int>();
      _ConvStrId2IntId(obj_ids_str, obj_ids_int);
      _coid_accesses.emplace(co_id, obj_ids_int);
    }
    Cons::P(boost::format("Loaded video access reqs from %d COs.") % _coid_accesses.size());
  }


  void _ConvStrId2IntId(const vector<string>& obj_ids_str, vector<int>* obj_ids_int) {
    map<string, int> objid_int;
    int idx = 0;
    for (auto& obj_id: obj_ids_str) {
      auto it = objid_int.find(obj_id);
      if (it == objid_int.end()) {
        objid_int.emplace(obj_id, idx);
        idx ++;
      }
    }

    for (auto& obj_id: obj_ids_str) {
      auto it = objid_int.find(obj_id);
      if (it == objid_int.end())
        THROW("Unexpected");
      obj_ids_int->push_back(it->second);
    }
  }


  void _WriteCondensed(const string& fn) {
    bf::path p(fn);
    string fn1 = str(boost::format("%s/%s") % Conf::DnOut() % p.filename().string());
    if (bf::exists(fn1))
      return;

    Cons::MT _(boost::format("Generating file %s") % fn1);
    ofstream ofs(fn1);
    size_t n = _coid_accesses.size();
    ofs.write((char*)&n, sizeof(n));
    for (auto i: _coid_accesses) {
      int co_id = i.first;
      ofs.write((char*)&co_id, sizeof(co_id));

      vector<int>* accesses = i.second;
      size_t n = accesses->size();
      ofs.write((char*)&n, sizeof(n));

      for (int obj_id: *accesses)
        ofs.write((char*)&obj_id, sizeof(obj_id));
    }
    ofs.close();
    Cons::P(boost::format("Created %s %d") % fn1 % boost::filesystem::file_size(fn1));
  }


  bool _LoadCondensed(const string& fn) {
    bf::path p(fn);
    string fn1 = str(boost::format("%s/%s") % Conf::DnOut() % p.filename().string());
    if (! bf::exists(fn1))
      return false;

    Cons::MT _(boost::format("Loading condensed YouTube video accesses by COs from %s ...") % fn1);

    int max_co_id = atoi(Conf::GetStr("max_co_id").c_str());

    ifstream ifs(fn1);
    size_t num_COs;
    ifs.read((char*)&num_COs, sizeof(num_COs));
    for (size_t i = 0; i < num_COs; i ++) {
      int co_id;
      ifs.read((char*)&co_id, sizeof(co_id));
      if (max_co_id != -1 && max_co_id < co_id) {
        Cons::P(boost::format("Passed max_co_id %d. Stop loading") % max_co_id);
        break;
      }

      size_t num_accesses;
      ifs.read((char*)&num_accesses, sizeof(num_accesses));
      vector<int>* obj_ids = new vector<int>();
      for (size_t j = 0; j < num_accesses; j ++) {
        int obj_id;
        ifs.read((char*)&obj_id, sizeof(obj_id));
        obj_ids->push_back(obj_id);
      }
      _coid_accesses.emplace(co_id, obj_ids);
    }
    Cons::P(boost::format("Loaded video access reqs from %d COs.") % _coid_accesses.size());
    return true;
  }


  void FreeMem() {
    for (auto i: _coid_accesses)
      delete i.second;
    _coid_accesses.clear();
  }


  const map<int, vector<int>* >& CoAccesses() {
    return _coid_accesses;
  }
}
