#pragma once

#include <string>


struct Coord {
  double lat;
  double lon;

  // 3D Cartesian coordinate on a sphere with a diameter 1
  double x;
  double y;
  double z;

  Coord(const double lat_, const double lon_);
};


struct CentralOffice {
  int id;
  Coord c;

  CentralOffice(int id, const Coord& c);

  std::string to_string();
};

std::ostream& operator<< (std::ostream& os, const CentralOffice& c);


namespace CentralOffices {
  void Load();
  // TODO: needed?
  CentralOffice* GetNearest(double lat, double lon);
  void FreeMem();
};
