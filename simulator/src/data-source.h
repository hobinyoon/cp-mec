#pragma once

#include <string>

#include "coord.h"


namespace DataSource {
  struct Node {
    int id;
    Coord c;

    Node(int id, const Coord& c);

    std::string to_string();
  };

  std::ostream& operator<< (std::ostream& os, const Node& c);


  void Load();
  Node* GetNearest(double lat, double lon);
  void FreeMem();
};
