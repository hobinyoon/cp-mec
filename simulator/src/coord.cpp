#include <cmath>

#include "coord.h"

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
