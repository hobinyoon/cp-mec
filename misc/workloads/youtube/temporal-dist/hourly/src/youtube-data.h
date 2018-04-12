#pragma once

#include <vector>
#include "op.h"

namespace YoutubeData {
	void Load();
	const std::vector<Op*>& Entries();
	size_t NumWrites();
	size_t NumReads();
	void FreeMem();
};
