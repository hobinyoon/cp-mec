#include <fstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <boost/regex.hpp>

#include "conf.h"
#include "cons.h"
#include "usa-map.h"
#include "util.h"

using namespace std;


namespace UsaMap {

  struct LatLon {
    double lat;
    double lon;

    LatLon(double lat_, double lon_)
      : lat(lat_), lon(lon_)
    { }

    bool operator!= (const LatLon& r) {
      return ((lat != r.lat) || (lon != r.lon));
    }
  };

  void GenSmallSizeData() {
    string fn = Conf::GetFn("usa_map");
    Cons::MT _(boost::format("Generating small size map from %s ...") % fn);

    size_t input_file_size = boost::filesystem::file_size(fn);

    const double dist_sq_threshold = atof(Conf::GetStr("dist_sq_threshold").c_str());
    vector<LatLon>* polygon = nullptr;
    vector<vector<LatLon>* > polygons;

    ifstream ifs(fn);
    for (string line; getline(ifs, line); ) {
      if ((line.size() == 0) || (line[0] == '#') || (line[0] == '\r') || (line[0] == '\n')) {
        // Finish a polygon
        if (polygon && (0 < polygon->size())) {
          //Cons::P("polygon_finished");
          polygons.push_back(polygon);
          polygon = nullptr;
        }
        continue;
      }

      //Cons::P(line);
      vector<string> t;
      static const auto sep = boost::is_any_of(" ");
      boost::split(t, line, sep);
      if (t.size() != 2)
        THROW(boost::format("Unexpected: %d [%s]") % t.size() % line);
      double lat = stod(t[0]);
      double lon = stod(t[1]);

      LatLon ll(lat, lon);
      if (polygon == nullptr) {
        polygon = new vector<LatLon>();
      }
      polygon->push_back(ll);
    }

    // Finish the last polygon
    if (polygon && (0 < polygon->size())) {
      //Cons::P("polygon_finished");
      polygons.push_back(polygon);
      polygon = nullptr;
    }

    size_t total_points = 0;
    for (auto p: polygons)
      total_points += p->size();

    Cons::P(boost::format("Loaded %d polygons. Average %f points per polygon.") % polygons.size()
        % (double(total_points) / polygons.size()));

    // Filter out the points that are too close to the preious one.
    vector<vector<LatLon>* > new_polygons;
    int num_points_filtered_out = 0;
    for (auto polygon: polygons) {
      vector<LatLon>* new_polygon = new vector<LatLon>();
      for (auto ll: *polygon) {
        if (new_polygon->size() == 0) {
          new_polygon->push_back(ll);
          continue;
        }

        const LatLon& ll_prev = *new_polygon->rbegin();
        double d_sq = (ll.lat - ll_prev.lat) * (ll.lat - ll_prev.lat)
          + (ll.lon - ll_prev.lon) * (ll.lon - ll_prev.lon);
        if (dist_sq_threshold < d_sq) {
          new_polygon->push_back(ll);
        } else {
          num_points_filtered_out ++;
        }
      }

      if (new_polygon->size() == 0)
        THROW("Unexpected");
      // The first and last points should match
      if (*(new_polygon->begin()) != *(new_polygon->rbegin()))
        new_polygon->push_back(*new_polygon->begin());

      new_polygons.push_back(new_polygon);
    }

    total_points = 0;
    for (auto p: new_polygons)
      total_points += p->size();

    Cons::P(boost::format("Generated %d polygons. Average %f points per polygon.") % new_polygons.size()
        % (double(total_points) / new_polygons.size()));

    // Delete old polygons
    for (auto i: polygons)
      delete i;
    polygons.clear();

    string fn_out = str(boost::format("%s/usa-map-smallsize-%f") % Conf::DnOut() % dist_sq_threshold);

    // Trim trailing 0s
    while (boost::regex_match(fn_out, boost::regex(".+\\.\\d+0+"))) {
      fn_out = fn_out.substr(0, fn_out.size() - 1);
    }

    ofstream ofs(fn_out);
    for (auto polygon: new_polygons) {
      for (auto p: *polygon) {
        ofs << p.lat << " " << p.lon << "\n";
      }
      ofs << "\n";
    }
    ofs.close();

    size_t output_file_size = boost::filesystem::file_size(fn_out);
    Cons::P(boost::format("created %s %d %f of input") % fn_out % output_file_size % (double(output_file_size) / input_file_size));

    // Delete new polygons
    for (auto i: new_polygons)
      delete i;
    new_polygons.clear();
  }
};
