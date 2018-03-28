#pragma once

#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "cons.h"
#include "util.h"

namespace Stat {
	template<class T> void Gen(const std::vector<T>& v1, const std::string& fn_cdf = std::string()) {
		if (v1.size() == 0) {
			Cons::P("No input to generate stat.");
			return;
		}

		std::vector<T> v(v1);
		sort(v.begin(), v.end());

		T  min = *(v.begin());
		T  max = *(v.rbegin());
		T  _1p = v[size_t(0.01 * (v.size() - 1))];
		T  _5p = v[size_t(0.05 * (v.size() - 1))];
		T _10p = v[size_t(0.10 * (v.size() - 1))];
		T _25p = v[size_t(0.25 * (v.size() - 1))];
		T _50p = v[size_t(0.50 * (v.size() - 1))];
		T _75p = v[size_t(0.75 * (v.size() - 1))];
		T _90p = v[size_t(0.90 * (v.size() - 1))];
		T _95p = v[size_t(0.95 * (v.size() - 1))];
		T _99p = v[size_t(0.99 * (v.size() - 1))];

		double sum = 0.0;
		double sum_sq = 0.0;
		for (T e: v) {
			sum += e;
			sum_sq += (e * e);
		}
		double avg = sum / v.size();
		double sd = sqrt(sum_sq / v.size() - avg * avg);
		std::string stat = str(boost::format(
					"avg %s" "\nsd  %s" "\nmin %s" "\nmax %s"
					"\n 1p %s" "\n 5p %s" "\n10p %s" "\n25p %s" "\n50p %s" "\n75p %s" "\n90p %s" "\n95p %s" "\n99p %s")
				% avg % sd % min % max
				% _1p % _5p % _10p % _25p % _50p % _75p % _90p % _95p % _99p
				);
		Cons::P(stat);

		if (fn_cdf != std::string()) {
			std::ofstream ofs(fn_cdf);
			if (! ofs.is_open())
				THROW(boost::format("Unable to open file %s") % fn_cdf);
			ofs << Util::Prepend("# ", stat);
			for (size_t i = 0; i < v.size(); i ++) {
				if (0 < i && i < (v.size() - 1) && v[i - 1] == v[i] && v[i] == v[i + 1])
					continue;
				ofs << v[i] << " " << (double(i) / v.size()) << "\n";
			}
			ofs << *v.rbegin() << " 1.0\n";
			ofs.close();
			Cons::P(boost::format("created a cdf file %s %d") % fn_cdf % boost::filesystem::file_size(fn_cdf));
		}
	}
};
