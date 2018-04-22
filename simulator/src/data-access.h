#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

// Data accesses in an EdgeDC
class DAs {
public:
  void AddSingleAccess(const std::string& line);
  void ConvStringObjToIntObj();

  void WriteCondensed(std::ofstream& ofs);
  void LoadCondensed(std::ifstream& ifs);

  size_t NumAccesses();
  int NumUsers();
  const std::vector<int>& ObjIds() const;

private:
  std::set<std::string> _user_ids;
  size_t _num_users = 0;

  // Obj accesses in the time order
  std::vector<std::string> _obj_ids_str;
  std::vector<int> _obj_ids;
};


namespace AllDAs {
  void AddSingleAccess(const int edc_id, const std::string& line);
  void ConvStringObjToIntObj();
  void FreeMem();
  const std::map<int, DAs* >& Get();
  const DAs* Get(int edc_id);
  void WriteCondensed(int edc_id, std::ofstream& ofs);
  void LoadCondensed(int edc_id, std::ifstream& ifs);
};
