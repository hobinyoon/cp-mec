#include <set>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "akamai-trace.h"
#include "cons.h"
#include "conf.h"
#include "util.h"

using namespace std;

namespace AkamaiTrace {
  namespace bf = boost::filesystem;

  void _GetFileList();
  void _GenCompactFiles();

  //vector<bf::path> _files;
  set<int> _file_ids;
  string _in_dn;


  void Gen() {
    _in_dn = Conf::Get("dn_access_trace");

    _GetFileList();
    _GenCompactFiles();
  }

  void _GetFileList() {
    for (auto i = bf::directory_iterator(_in_dn); i != bf::directory_iterator(); i++) {
      auto p = i->path();
      string s = p.filename().string();

      if (s.size() == 0)
        continue;
      if (! isdigit(s[0]))
        continue;

      _file_ids.insert(stoi(p.filename().string()));
      //Cons::P(p.string());
      //Cons::P(p.filename().string());
    }
    Cons::P(boost::format("Found %d data access trace files") % _file_ids.size());
  }

  struct Item {
    long ts;
    long obj_id;

    Item(long ts_, long obj_id_)
      : ts(ts_), obj_id(obj_id_)
    {}

    void Write(ofstream& ofs) {
      ofs.write((char*)&ts, sizeof(ts));
      ofs.write((char*)&obj_id, sizeof(obj_id));
    }
  };

  void _GenCompactFiles() {
    for (int fid: _file_ids) {
      string out_fn = str(boost::format("%s/%d") % Conf::DnOut() % fid);
      if (bf::exists(out_fn))
        continue;

      string in_fn = str(boost::format("%d/%f") % _in_dn % fid);
      vector<Item> items;
      {
        Cons::MT _(boost::format("Reading %d %.1fMB") % fid % (boost::filesystem::file_size(in_fn) / 1024.0 / 1024));

        set<string> uniq_obj_ids;
        map<string, long> strid_intid;
        ifstream ifs(in_fn);
        for (string line; getline(ifs, line); ) {
          static const auto sep = boost::is_any_of("\t");
          vector<string> t;
          boost::split(t, line, sep, boost::token_compress_on);
          if (t.size() != 7)
            THROW(boost::format("Unexpected: [%s]") % line);
          // 1475531422	66.81.109.120	1091	1740	ff430ce4de44677458914d41c2c25776d4c35ce8898bbd9fff8d32272eb470d4	200	0
          long ts = stol(t[0]);
          const string& obj_id = t[4];

          auto it = uniq_obj_ids.find(obj_id);
          if (it == uniq_obj_ids.end()) {
            uniq_obj_ids.insert(obj_id);
            strid_intid.emplace(obj_id, uniq_obj_ids.size());
          }
          items.push_back(Item(ts, strid_intid[obj_id]));

          if (items.size() % 10000 == 0) {
            Cons::ClearLine();
            Cons::Pnnl(boost::format("Read %d K items") % (items.size() / 1000));
          }
        }
        Cons::ClearLine();
        Cons::P(boost::format("Read %d items") % items.size());

        Cons::P(boost::format("%d %d items. %d uniq items.") % fid % items.size() % uniq_obj_ids.size());
      }

      {
        Cons::MT _(boost::format("Creating %s") % out_fn);

        ofstream ofs(out_fn);
        size_t n = items.size();
        ofs.write((char*)&n, sizeof(n));
        for (auto i: items)
          i.Write(ofs);
        ofs.close();
        Cons::P(boost::format("Created %s %d") % out_fn % boost::filesystem::file_size(out_fn));
      }
    }
  }
};
