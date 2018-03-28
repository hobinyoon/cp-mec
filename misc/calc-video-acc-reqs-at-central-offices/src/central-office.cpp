#include <chrono>
#include <fstream>
#include <random>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "conf.h"
#include "cons.h"
#include "central-office.h"
#include "stat.h"

using namespace std;


CentralOffice::CentralOffice(double lat_, double lon_)
: lat(lat_), lon(lon_)
{
}

string CentralOffice::to_string() {
	return str(boost::format("%f %f") % lat % lon);
}
//string to_string(const CentralOffice& c) {
//	return str(boost::format("%s %f %f") % c.name % c.lat % c.lon);
//}

//const Op* CentralOffice::ReadObj(const Op* op_read, double& latency) {
//	const Op* op = _cache.Get(op_read);
//	if (op != NULL)
//		return op;
//
//	// When cache miss, contact the nearest edge data center.
//	//
//	// Note: This always returns the same result (thus the read paths form a
//	// tree) and can be cached.
//	//
//	// Idea: You could do cooperative caching by querying near edge data centers.
//	// The location can be globally maintained with a DHT or if the object is big
//	// enough, the location metadata could be worth keeping in a global database.
//	// There are many cooperative caching techniques too.
//
//	EdgeDc* e_dc = EdgeDCs::GetNearest(lat, lon);
//	latency += Latency::Wired(lat, lon, e_dc->lat, e_dc->lon);
//	const Op* op1 = e_dc->ReadObj(op_read, latency);
//	_cache.Put(op1, op_read->created_at);
//	return op1;
//}


namespace CentralOffices {
	static map<int, CentralOffice*> _COs;
  
  // TODO: let's do 2D indexing first and move on to 3D
  namespace bg = boost::geometry;
  namespace bgi = boost::geometry::index;
  typedef bg::model::point<double, 2, bg::cs::cartesian> point;
  typedef std::pair<point, int> value;

  bgi::rtree<value, bgi::quadratic<16> > rtree;

	void Init() {
		string fn = Conf::GetFn("central_office_locations");
		Cons::MT _(boost::format("Loading central office locations from %s ...") % fn);

    // TODO: When the file doesn't exist, unzip from .7z.

    // Interesting. The file contains many duplicate items. Let's filter out duplicates.
    //
    // TODO: double check with just command line processing.

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
    int id = 0;
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

			_COs.emplace(id, new CentralOffice(lat, lon));

      rtree.insert(std::make_pair(point(lon, lat), id));

      id ++;
		}
		Cons::P(boost::format("loaded %d. Ignored %d out-of-range, %d dup-coord points.") % _COs.size() % num_oor % num_dups);
    if (false) {
      for (const auto i: _COs)
        Cons::P(i.second->to_string());
    }

    if (false) {
      // Nearest point query. Seems to be working.
      
      // Somewhere in Home Park
      point p1(-84.398932, 33.788164);
      std::vector<value> result_n;
      rtree.query(bgi::nearest(p1, 1), std::back_inserter(result_n));
      if (result_n.size() != 1)
        THROW("Unexpected");
      value r = result_n[0];
      double r_lon = bg::get<0>(r.first);
      double r_lat = bg::get<1>(r.first);
      Cons::P(boost::format("%f %f %d") % r_lat % r_lon % r.second);
    }
	}

	CentralOffice* GetNearest(double lat, double lon) {
		if (_COs.size() == 0) {
			return nullptr;
		} else {
      // TODO
			//return _COs[_idx.GetNearest(lat, lon)];
			return nullptr;
		}
	}

//	void ReportCacheStat() {
//		Cons::MT _("Micro edge DCs cache stat:", false);
//
//		vector<long> hits;
//		vector<long> misses;
//		vector<double> hit_ratios;
//		vector<long> num_items;
//		for (auto dc: _COs) {
//			auto cs = dc->CacheStat();
//			hits.push_back(cs.hits);
//			misses.push_back(cs.misses);
//			if (cs.hits + cs.misses > 0)
//				hit_ratios.push_back(100.0 * cs.hits / (cs.hits + cs.misses));
//			num_items.push_back(cs.num_items);
//		}
//
//		Cons::P(boost::format("%d micro edge DCs") % _COs.size());
//
//		{
//			Cons::MT _("Cache hit ratio (%%):", false);
//			Stat::Gen<double>(hit_ratios,
//					str(boost::format("%s/micro-edge-dc-cache-hit-ratios-cdf") % Conf::OutputDir()));
//		}
//		{
//			Cons::MT _("Cache size (number of items):", false);
//			Stat::Gen<long>(num_items,
//					str(boost::format("%s/micro-edge-dc-num-items-cached-cdf") % Conf::OutputDir()));
//		}
//	}
};
