#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#include "conf.h"
#include "cons.h"
#include "data-source.h"
#include "util.h"

using namespace std;

namespace DataSource {
  Node::Node(int id_, const Coord& c_)
    : id(id_), c(c_)
  {
  }

  string Node::to_string() {
    return str(boost::format("%d %f %f") % id % c.lat % c.lon);
  }

  ostream& operator<< (ostream& os, const Node& c) {
    os << c.id << " " << c.c.lat << " " << c.c.lon;
    return os;
  }


  namespace bf = boost::filesystem;

  static vector<Node*> _nodes;

  namespace bg = boost::geometry;
  namespace bgi = boost::geometry::index;
  typedef bg::model::point<double, 3, bg::cs::cartesian> P3;
  typedef std::pair<P3, int> P3_id;

  // Point index
  bgi::rtree<P3_id, bgi::quadratic<16> > _rtree;


  void Load() {
    string fn = Conf::GetFn("data_source_locations");
    Cons::MT _(boost::format("Loading data source locations from %s ...") % fn);

    if (! bf::exists(fn))
      THROW("Unexpected");

    ifstream ifs(fn);
    int loc_id = 0;
    for (string line; getline(ifs, line); ) {
      if (line.size() == 0 || line[0] == '#')
        continue;
      static const auto sep = boost::is_any_of(" ");
      vector<string> t;
      boost::split(t, line, sep, boost::token_compress_on);
      if (t.size() != 2)
        THROW(boost::format("Unexpected: %d") % t.size());
      double lat = stod(t[0]);
      double lon = stod(t[1]);

      Coord coord(lat, lon);
      _nodes.push_back(new Node(loc_id, coord));
      _rtree.insert(std::make_pair(P3(coord.x, coord.y, coord.z), loc_id));
      loc_id ++;
    }
    Cons::P(boost::format("loaded %d nodes.") % _nodes.size());
    if (false) {
      for (const auto i: _nodes)
        Cons::P(i->to_string());
    }
  }


  Node* GetNearest(double lat, double lon) {
    if (_nodes.size() == 0) {
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
      return _nodes[r.second];
    }
  }

  void FreeMem() {
    for (const auto i: _nodes)
      delete i;
    _nodes.clear();
  }
};
