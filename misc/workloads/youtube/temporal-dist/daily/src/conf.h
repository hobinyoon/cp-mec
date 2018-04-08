#pragma once

#include <string>

namespace Conf {
	extern std::string dn_plot_data;
	extern std::string fn_plot_data;

	void Init(int argc, char* argv[]);

	const std::string Get(const std::string& k);
};
