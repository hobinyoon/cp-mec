#pragma once

#include <string>

struct CentralOffice {
	double lat;
	double lon;

	CentralOffice(double lat, double lon);

	std::string to_string();
};


namespace CentralOffices {
	void Init();
	CentralOffice* GetNearest(double lat, double lon);
};
