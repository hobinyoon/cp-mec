#pragma once

#include <map>
#include <string>
#include <vector>

namespace YoutubeAccess {
  void Load();
  void FreeMem();

  bool CoHasAccesses(int co_id);
  const std::map<int, std::vector<std::string>* >& CoAccesses();
};
