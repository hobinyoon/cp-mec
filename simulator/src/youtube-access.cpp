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

  Accesses::Accesses(const vector<string>& obj_ids_str, int num_users) {
    // Convert the string obj IDs to integer ones
    {
      map<string, int> objid_int;
      int idx = 0;
      for (auto& obj_id: obj_ids_str) {
        auto it = objid_int.find(obj_id);
        if (it == objid_int.end()) {
          objid_int.emplace(obj_id, idx);
          idx ++;
        }
      }

      _obj_ids = new vector<int>();
      for (auto& obj_id: obj_ids_str) {
        auto it = objid_int.find(obj_id);
        if (it == objid_int.end())
          THROW("Unexpected");
        _obj_ids->push_back(it->second);
      }
    }

    _num_users = num_users;
  }

  Accesses::Accesses(vector<int>* obj_id_accessed_, int num_users) {
    _obj_ids = obj_id_accessed_;
    _num_users = num_users;
  }

  Accesses::~Accesses() {
    if (_obj_ids)
      delete _obj_ids;
  }

  size_t Accesses::NumAccesses() {
    return _obj_ids->size();
  }

  int Accesses::NumUsers() {
    return _num_users;
  }

  std::vector<int>* Accesses::ObjIds() const {
    return _obj_ids;
  }


  map<int, Accesses*> _elid_accesses;

  void _Load(const string& fn);
  void _WriteCondensed(const string& fn);
  bool _LoadCondensed(const string& fn);


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
    Cons::MT _(boost::format("Loading YouTube video accesses by edge locations from %s ...") % fn);

    int max_el_id = atoi(Conf::Get("max_el_id").c_str());

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

      int el_id = atoi(t[0].c_str());
      if (max_el_id != -1 && max_el_id < el_id) {
        Cons::P(boost::format("Passed max_el_id %d. Stop loading") % max_el_id);
        break;
      }

      int num_accesses = atoi(t[3].c_str());
      if (num_accesses <= 1) {
        THROW(boost::format("Unexpected [%s]") % line);
        continue;
      }

      vector<string> obj_ids_str;
      set<string> user_ids;
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

        user_ids.insert(t[1]);
        obj_ids_str.push_back(t[6]);
      }

      Accesses* accesses = new Accesses(obj_ids_str, user_ids.size());
      _elid_accesses.emplace(el_id, accesses);
    }
    Cons::P(boost::format("Loaded obj access reqs from %d edge locations.") % _elid_accesses.size());
  }


  void _WriteCondensed(const string& fn) {
    bf::path p(fn);
    string fn1 = str(boost::format("%s/%s") % Conf::DnOut() % p.filename().string());
    if (bf::exists(fn1))
      return;

    Cons::MT _(boost::format("Generating file %s") % fn1);
    ofstream ofs(fn1);
    size_t n = _elid_accesses.size();
    ofs.write((char*)&n, sizeof(n));
    for (auto i: _elid_accesses) {
      int el_id = i.first;
      ofs.write((char*)&el_id, sizeof(el_id));

      Accesses* acc = i.second;
      {
        int n = acc->NumUsers();
        ofs.write((char*)&n, sizeof(n));
      }
      {
        size_t n = acc->NumAccesses();
        ofs.write((char*)&n, sizeof(n));
      }

      for (int obj_id: *(acc->ObjIds()))
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

    Cons::MT _(boost::format("Loading condensed YouTube video accesses by edge locations from %s ...") % fn1);

    int max_el_id = atoi(Conf::Get("max_el_id").c_str());

    ifstream ifs(fn1);
    size_t num_ELs;
    ifs.read((char*)&num_ELs, sizeof(num_ELs));
    for (size_t i = 0; i < num_ELs; i ++) {
      int el_id;
      ifs.read((char*)&el_id, sizeof(el_id));
      if (max_el_id != -1 && max_el_id < el_id) {
        Cons::P(boost::format("Passed max_el_id %d. Stop loading") % max_el_id);
        break;
      }

      int num_users;
      ifs.read((char*)&num_users, sizeof(num_users));

      size_t num_accesses;
      ifs.read((char*)&num_accesses, sizeof(num_accesses));
      vector<int>* obj_ids = new vector<int>();
      for (size_t j = 0; j < num_accesses; j ++) {
        int obj_id;
        ifs.read((char*)&obj_id, sizeof(obj_id));
        obj_ids->push_back(obj_id);
      }

      _elid_accesses.emplace(el_id, new Accesses(obj_ids, num_users));
    }
    Cons::P(boost::format("Loaded video access reqs from %d edge locations.") % _elid_accesses.size());
    return true;
  }


  void FreeMem() {
    for (auto i: _elid_accesses)
      delete i.second;
    _elid_accesses.clear();
  }


  const map<int, Accesses* >& ElAccesses() {
    return _elid_accesses;
  }
}
