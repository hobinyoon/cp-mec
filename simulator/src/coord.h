#pragma once

struct Coord {
  double lat;
  double lon;

  // 3D Cartesian coordinate on a sphere with a diameter 1
  double x;
  double y;
  double z;

  Coord(const double lat_, const double lon_);
};
