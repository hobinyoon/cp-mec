#include <fstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/regex.hpp>

#include "conf.h"
#include "cons.h"
#include "central-office.h"
#include "util.h"

using namespace std;


namespace CentralOffices {
  namespace bg = boost::geometry;
  namespace bgi = boost::geometry::index;
  // For hiding points, use 2D distance since the points will be plotted in a 2D map.
  typedef bg::model::point<double, 2, bg::cs::cartesian> P2;
  typedef std::pair<P2, int> P2_id;

  // Point index
  bgi::rtree<P2_id, bgi::quadratic<16> > _rtree;

  double GetDistToNearest(double lat, double lon);

  struct LatLon {
    double lat;
    double lon;

    LatLon(double lat_, double lon_)
      : lat(lat_), lon(lon_)
    { }
  };

  void GenReducedData() {
    string dn_base = boost::filesystem::path(__FILE__).parent_path().string();
    string fn = str(boost::format("%s/../%s") % dn_base % Conf::GetStr("central_office_locations"));

    Cons::MT _(boost::format("Loading central office locations from %s ...") % fn);

    const double dist_sq_threshold = atof(Conf::GetStr("dist_sq_threshold").c_str());
    vector<LatLon> sparse_points;

    ifstream ifs(fn);
    int co_id = 0;
    int num_COs_filtered_out = 0;
    for (string line; getline(ifs, line); ) {
      if (line.size() == 0)
        continue;
      if (line[0] == '#')
        continue;

      //Cons::P(line);
      vector<string> t;
      static const auto sep = boost::is_any_of(" ");
      boost::split(t, line, sep);
      if (t.size() != 2)
        THROW(boost::format("Unexpected: %d") % t.size());
      double lat = stod(t[0]);
      double lon = stod(t[1]);

      double d = GetDistToNearest(lat, lon);

      if (d == -1.0 || dist_sq_threshold < d) {
        _rtree.insert(std::make_pair(P2(lon, lat), co_id));
        sparse_points.push_back(LatLon(lat, lon));
      } else {
        num_COs_filtered_out ++;
      }
      co_id ++;
    }

    // Print out the sparse points
    //   Couldn't figure out how to print out the points in _rtree. Tried all of these:
    //     https://stackoverflow.com/questions/27037129/how-to-iterate-over-a-boost-r-tree
    Cons::P(boost::format("%d points after dropping %d almost duplicate points") % sparse_points.size() % num_COs_filtered_out);

    string fn_out = str(boost::format("%s/centraloffices-wo-almost-dup-points-%f") % Conf::DnOut() % dist_sq_threshold);

    // Trim trailing 0s
    while (boost::regex_match(fn_out, boost::regex(".+\\.\\d+0+"))) {
      fn_out = fn_out.substr(0, fn_out.size() - 1);
    }

    ofstream ofs(fn_out);
    for (auto i: sparse_points) {
      ofs << i.lat << " " << i.lon << "\n";
    }
    ofs.close();
    Cons::P(boost::format("created %s %d") % fn_out % boost::filesystem::file_size(fn_out));
  }

  // Returns
  //   dist^2 to the closest point when there is at least one point in _rtree
  //   -1 when there is no point in _rtree
  double GetDistToNearest(double lat, double lon) {
    P2 p1(lon, lat);
    std::vector<P2_id> result_n;
    _rtree.query(bgi::nearest(p1, 1), std::back_inserter(result_n));
    if (result_n.size() == 0)
      return -1.0;
    if (result_n.size() != 1)
      THROW("Unexpected");
    P2_id r = result_n[0];
    double r_x = bg::get<0>(r.first);
    double r_y = bg::get<1>(r.first);
    return (r_x - lon) * (r_x - lon)
      + (r_y - lat) * (r_y - lat);
  }
};
