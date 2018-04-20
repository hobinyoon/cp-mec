#pragma once

#include <map>
#include <string>
#include <vector>

namespace YoutubeAccess {
  void Load();
  void FreeMem();

  // Accesses at an edge location
  class Accesses {
    // Obj IDs accesses in the time order
    std::vector<int>* _obj_ids;
    int _num_users;

  public:
    Accesses(const std::vector<std::string>& obj_ids_str, int num_users);
    Accesses(std::vector<int>* obj_ids, int num_users);
    ~Accesses();

    size_t NumAccesses();
    int NumUsers();
    std::vector<int>* ObjIds() const;
  };

  const std::map<int, Accesses* >& ElAccesses();
};
