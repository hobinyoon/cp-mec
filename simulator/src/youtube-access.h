#pragma once

#include <map>
#include <string>
#include <vector>

namespace YoutubeAccess {
  void Load();
  void FreeMem();

  const std::map<int, std::vector<int>* >& CoAccesses();
};
