#pragma once

#include <iostream>
#include <string>

struct Op {
	double lat;
	double lon;
	std::string obj_id;
	//boost::posix_time::ptime created_at;
	std::string created_at;

	virtual ~Op();

	virtual std::string to_string() const = 0;
};


// Needed by <set> or <map>
// bool operator< (const Op& a, const Op& b);
