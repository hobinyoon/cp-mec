#include <fstream>
#include <mutex>
#include <thread>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "conf.h"
#include "cons.h"
#include "data-access.h"
#include "edc-da-loader.h"
#include "edge-dc.h"
#include "util.h"

using namespace std;

namespace EdcDaLoader {
  namespace bf = boost::filesystem;

  bool _LoadCondensed(const string& fn);
  void _LoadRaw(const string& fn);

  string _fn_condensed;


  void Load() {
    string fn = Conf::GetFn("video_accesses_by_COs");
    if (! bf::exists(fn))
      THROW("Unexpected");

    if (_LoadCondensed(fn))
      return;
    _LoadRaw(fn);
    EdgeDCs::WriteCondensed(_fn_condensed);
  }


  void _LoadRaw(const string& fn) {
    Cons::MT _(boost::format("Loading edge DCs and their data accesses from %s ...") % fn);

    int max_el_id = stoi(Conf::Get("max_el_id"));
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

      EdgeDC* edc = EdgeDCs::Add(t);
      int edc_id = edc->Id();
      if (max_el_id != -1 && max_el_id < edc_id) {
        Cons::P(boost::format("Passed max_el_id %d. Stop loading") % max_el_id);
        EdgeDCs::Delete(edc_id);
        break;
      }

      int num_accesses = stoi(t[3]);
      for (int i = 0; i < num_accesses; i ++) {
        if (! getline(ifs, line))
          THROW(boost::format("Unexpected [%s]") % line);
        AllDAs::AddSingleAccess(edc_id, line);
      }
    }

    AllDAs::ConvStringObjToIntObj();
    Cons::P(boost::format("Loaded obj access reqs from %d edge DCs.") % EdgeDCs::NumDCs());
  }


  bool _LoadCondensed(const string& fn) {
    bf::path p(fn);
    _fn_condensed = str(boost::format("%s/%s") % Conf::DnOut() % p.filename().string());
    if (! bf::exists(_fn_condensed))
      return false;
    EdgeDCs::LoadCondensed(_fn_condensed);
    return true;
  }
}
