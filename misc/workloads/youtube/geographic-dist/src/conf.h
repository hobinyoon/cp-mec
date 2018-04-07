#pragma once

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <yaml-cpp/yaml.h>

namespace Conf {
	extern std::string dn_plot_data;
	extern std::string fn_plot_data;

	void Init(int argc, char* argv[]);

	YAML::Node Get(const std::string& k);
	std::string GetString(const std::string& k);

	const std::string Desc();
};
