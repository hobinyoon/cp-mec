#include <chrono>
#include <fstream>
#include <random>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "conf.h"
#include "cons.h"
#include "central-office.h"
#include "stat.h"

using namespace std;


CentralOffice::CentralOffice(int id_, const Coord& c_)
  : id(id_), c(c_)
{
}

string CentralOffice::to_string() {
  return str(boost::format("%d %f %f") % id % c.lat % c.lon);
}

ostream& operator<< (ostream& os, const CentralOffice& c) {
  os << c.id << " " << c.c.lat << " " << c.c.lon;
  return os;
}


Coord::Coord(const double lat_, const double lon_)
  : lat(lat_), lon(lon_)
{
  // http://stackoverflow.com/questions/1185408/converting-from-longitude-latitude-to-cartesian-coordinates
  double lat_r = M_PI * (lat / 180);
  double lon_r = M_PI * (lon / 180);
  x = cos(lat_r) * cos(lon_r);
  y = cos(lat_r) * sin(lon_r);
  z = sin(lat_r);
}


namespace CentralOffices {
  static vector<CentralOffice*> _COs;

  namespace bg = boost::geometry;
  namespace bgi = boost::geometry::index;
  typedef bg::model::point<double, 3, bg::cs::cartesian> P3;
  typedef std::pair<P3, int> P3_id;

  // Point index
  bgi::rtree<P3_id, bgi::quadratic<16> > _rtree;

  void Load() {
    string fn = Conf::GetFn("central_office_locations");
    Cons::MT _(boost::format("Loading central office locations from %s ...") % fn);

    // Unzip if needed
    if (! boost::filesystem::exists(fn)) {
      string fn_7z = fn + ".7z";
      if (boost::filesystem::exists(fn_7z)) {
        string dn = boost::filesystem::path(fn).parent_path().string();
        string fn1 = boost::filesystem::path(fn).filename().string();
        string cmd = str(boost::format("cd %s && 7z e %s.7z") % dn % fn1);
        Util::RunSubprocess(cmd);
      } else
        THROW("Unexpected");
    }

    struct P2 {
      double x;
      double y;

      P2(const double x_, const double y_)
        : x(x_), y(y_)
      {}

      bool operator < (const P2& r) const {
        if (x < r.x) return true;
        else if (x > r.x) return false;

        return (y < r.y);
      }
    };

    set<P2> uniq_points;

    ifstream ifs(fn);
    int co_id = 0;
    int num_dups = 0;
    int num_oor = 0;
    for (string line; getline(ifs, line); ) {
      static const auto sep = boost::is_any_of(" ");
      vector<string> t;
      boost::split(t, line, sep);
      if (t.size() != 2)
        THROW(boost::format("Unexpected: %d") % t.size());
      double lat = stod(t[0]);
      double lon = stod(t[1]);

      // Filter out incorrect points and duplicates points.
      //   Interesting there are quite a lot of them.

      if ((lat <= 0) || (90 <= lat)
          || (lon <= -180) || (0 <= lon)) {
        //Cons::P(boost::format("Ignoring out-of-range point: [%s]") % line);
        num_oor ++;
        continue;
      }

      {
        auto it = uniq_points.find(P2(lat, lon));
        if (it != uniq_points.end()) {
          //Cons::P(boost::format("Ignoring dup point: %f %f [%s]") % lat % lon % line);
          num_dups ++;
          continue;
        }
        uniq_points.insert(P2(lat, lon));
      }

      Coord coord(lat, lon);
      _COs.push_back(new CentralOffice(co_id, coord));
      _rtree.insert(std::make_pair(P3(coord.x, coord.y, coord.z), co_id));
      co_id ++;
    }
    Cons::P(boost::format("loaded %d. Ignored %d out-of-range, %d dup-coord points.") % _COs.size() % num_oor % num_dups);
    if (false) {
      for (const auto i: _COs)
        Cons::P(i->to_string());
    }

    if (true) {
      // Test the nearest point query. Seems to work.

      // Somewhere in Home Park
      if (true) {
        CentralOffice* co = GetNearest(33.788164, -84.398932);
        if (co == nullptr)
          THROW("Unexpected");
        Cons::P(boost::format("%d %f %f") % co->id % co->c.lat % co->c.lon);
      }
    }
  }

  CentralOffice* GetNearest(double lat, double lon) {
    if (_COs.size() == 0) {
      return nullptr;
    } else {
      Coord c1(lat, lon);
      P3 p1(c1.x, c1.y, c1.z);
      std::vector<P3_id> result_n;
      _rtree.query(bgi::nearest(p1, 1), std::back_inserter(result_n));
      if (result_n.size() != 1)
        THROW("Unexpected");
      P3_id r = result_n[0];
      //double r_x = bg::get<0>(r.first);
      //double r_y = bg::get<1>(r.first);
      //double r_z = bg::get<2>(r.first);
      return _COs[r.second];
    }
  }

  void FreeMem() {
    for (const auto i: _COs)
      delete i;
    _COs.clear();
  }
};
