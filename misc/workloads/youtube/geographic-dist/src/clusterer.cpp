#include <fstream>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/regex.hpp>

#include "conf.h"
#include "cons.h"
#include "util.h"
#include "youtube-data.h"

using namespace std;


// Moving to boost R-tree.
//   nanoflann doesn't seem to have incremental update interface.
//   Boost R-tree is Quite fast.
//     Clustering point clusters ... 100.00% from   54699 to    2618. 95.21% reduction
//       1037 ms
//     Clustering point clusters ... 100.00% from    2618 to    2618.  0.00% reduction
//        152 ms
namespace Clusterer {
  namespace bg = boost::geometry;
  namespace bgi = boost::geometry::index;
  typedef bg::model::point<double, 2, bg::cs::cartesian> point;
  typedef std::pair<point, size_t> value;

  class PointCluster {
    private:
      vector<point> points;
      // center is a point object, not a pointer to it, since it can be one not
      // from "points".
      point center;

    public:
      PointCluster() {
      }

      PointCluster(const point& p) {
        points.push_back(p);
        center = p;
      }

      PointCluster(const PointCluster& pc) {
        if (pc.points.size() == 0)
          THROW("Unexpected");

        points = pc.points;
        center = pc.center;
      }

      ~PointCluster() {
      }

      void Add(const PointCluster& pc) {
        points.insert(end(points), begin(pc.points), end(pc.points));
      }

      point Center() {
        if (points.size() == 0)
          THROW("Unexpected");
        return center;
      }

      size_t Size() const {
        return points.size();
      }

      void RecalcCenter() {
        if (points.size() == 0)
          THROW("Unexpected");

        double sum_x = 0.0;
        double sum_y = 0.0;
        for (auto p: points) {
          sum_x += bg::get<0>(p);
          sum_y += bg::get<1>(p);
        }
        center = point(sum_x / points.size(), sum_y / points.size());
      }
  };

  vector<PointCluster> _ClusterPointClusters(vector<PointCluster>& pcs);

  void Run() {
    YoutubeData::Load();

    // Init point clusters with each cluster consisting of just one point.
    // Point clusters
    vector<PointCluster> pcs;
    for (auto op: YoutubeData::Entries())
      pcs.push_back(PointCluster(point(op->lon, op->lat)));

    // Make multiple paths, until it stabilizes
    while (true) {
      vector<PointCluster> pcs1 = _ClusterPointClusters(pcs);
      size_t pcs_size = pcs.size();
      size_t pcs1_size = pcs1.size();

      pcs = pcs1;
      if (pcs_size == pcs1_size)
        break;
    }

    const string& fn = Conf::fn_plot_data;
    ofstream ofs(fn);
    ofs << "# lon lat num_points_clustered\n";

    size_t max_cluster_size = 0;
    for (auto pc: pcs)
      max_cluster_size = max(max_cluster_size, pc.Size());
    ofs << boost::format("# max_cluster_size=%d\n") % max_cluster_size;

    // Sort by the cluster size. Helps big clusters more noticeable.
    sort(pcs.begin(), pcs.end(),
        [](const PointCluster & a, const PointCluster & b) -> bool
        {
          return a.Size() < b.Size();
        });

    for (auto pc: pcs) {
      const point& c = pc.Center();
      double x = bg::get<0>(c);
      double y = bg::get<1>(c);
      ofs << x << " " << y << " " << pc.Size() << "\n";
    }
    ofs.close();
    Cons::P(boost::format("Created %s %d") % fn % boost::filesystem::file_size(fn));
    Cons::P(boost::format("Max point cluster size: %d") % max_cluster_size);
  }

  vector<PointCluster> _ClusterPointClusters(vector<PointCluster>& pcs) {
    Cons::MTnnl _("Clustering point clusters ... ");

    vector<PointCluster> new_pcs;
    bgi::rtree<value, bgi::quadratic<16> > rtree;
    double dist_sq_threshold = Conf::Get("dist_sq_threshold").as<double>();

    size_t i = 0;
    for (auto pc: pcs) {
      if (new_pcs.size() == 0) {
        PointCluster pc1(pc);
        new_pcs.push_back(pc1);
        rtree.insert(std::make_pair(pc1.Center(), new_pcs.size() - 1));
      } else {
        point c = pc.Center();
        double c_x = bg::get<0>(c);
        double c_y = bg::get<1>(c);

        std::vector<value> result_n;
        rtree.query(bgi::nearest(c, 1), std::back_inserter(result_n));
        if (result_n.size() != 1)
          THROW("Unexpected");
        const point& n_pc_c = result_n[0].first;
        size_t n_pc_idx = result_n[0].second;
        double n_pc_c_x = bg::get<0>(n_pc_c);
        double n_pc_c_y = bg::get<1>(n_pc_c);

        double dist_sq = (c_x - n_pc_c_x) * (c_x - n_pc_c_x)
          + (c_y - n_pc_c_y) * (c_y - n_pc_c_y);

        // Make a new point cluster when it's far enough from each of the
        // center of the point clusters
        if (dist_sq_threshold < dist_sq) {
          PointCluster pc1(pc);
          new_pcs.push_back(pc1);
          rtree.insert(std::make_pair(pc1.Center(), new_pcs.size() - 1));
        } else {
          //Add the point cluster to the existing cluster
          new_pcs[n_pc_idx].Add(pc);
        }
      }

      // Progress report
      if (i % 1000 == 0) {
        if (i != 0) {
          for (int j = 0; j < 7; j ++)
            Cons::EraseChar();
        }
        cout << boost::format("%6.2f%%") % (100.0 * i / pcs.size()) << flush;
      }

      i ++;
    }

    for (int j = 0; j < 7; j ++)
      Cons::EraseChar();
    cout << boost::format("%.2f%%") % (100.0 * i / pcs.size());

    cout << boost::format(" from %7d to %7d. %5.2f%% reduction\n")
      % pcs.size()
      % new_pcs.size()
      % (100.0 * (pcs.size() - new_pcs.size()) / pcs.size());

    // Re-calculate the centers
    for (auto& pc: new_pcs)
      pc.RecalcCenter();

    return new_pcs;
  }
};
