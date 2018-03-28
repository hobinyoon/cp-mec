#pragma once

#include <iostream>
#include <string>

struct Op {
	enum Type {
		NA = 0,	// not assigned
		W = 1,
		R = 2,
	};

	Type type;
	double lat;
	double lon;
	std::string obj_id;

	// The request time of the operation: write time or read time.
	//boost::posix_time::ptime created_at;
	std::string created_at;

	char in_usa;

	virtual ~Op();

	virtual std::string to_string() const = 0;
};


// Needed by <set> or <map>
// bool operator< (const Op& a, const Op& b);


std::ostream& operator<< (std::ostream& os, const Op::Type& opt);
