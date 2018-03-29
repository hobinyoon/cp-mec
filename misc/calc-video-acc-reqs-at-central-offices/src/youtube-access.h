#pragma once

#include <vector>
#include "op.h"

namespace YoutubeAccess {
  void Load();
  const std::vector<Op*>& Entries();
  size_t NumWrites();
  size_t NumReads();
  void FreeMem();
};
