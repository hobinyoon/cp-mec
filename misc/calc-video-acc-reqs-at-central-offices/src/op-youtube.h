#pragma once

#include <iostream>
#include <string>
#include "op.h"

struct OpYoutube : public Op {
	// Not loading fields that are not used in this simulation to save memory and
	// time.
	//
	//long id;	// tweet id
	//long uid;
	//std::string created_at_str;
	// Op::obj_id is youtube_video_id
	//long youtube_video_uploader;
	//std::vector<std::string> topics;
	OpYoutube(std::ifstream& ifs);

	std::string to_string() const;
};

std::ostream& operator<< (std::ostream& os, const OpYoutube& op);
