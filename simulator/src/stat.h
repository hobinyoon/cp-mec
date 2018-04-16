#pragma once

#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "cons.h"
#include "util.h"

namespace Stat {
  template<class T>
  struct Result {
    T  min;
    T  max;
    T  _1p;
    T  _5p;
    T _10p;
    T _25p;
    T _50p;
    T _75p;
    T _90p;
    T _95p;
    T _99p;
    double sum;
    double avg;
    double sd;
  };

  template<class T>
  Result<T> Gen(const std::vector<T>& v1, const std::string& fn_cdf = std::string()) {
    if (v1.size() == 0) {
      THROW("No input to generate stat");
    }

    std::vector<T> v(v1);
    sort(v.begin(), v.end());

    Result<T> r;
    r.min  = *(v.begin());
    r.max  = *(v.rbegin());
    r._1p  = v[size_t(0.01 * (v.size() - 1))];
    r._5p  = v[size_t(0.05 * (v.size() - 1))];
    r._10p = v[size_t(0.10 * (v.size() - 1))];
    r._25p = v[size_t(0.25 * (v.size() - 1))];
    r._50p = v[size_t(0.50 * (v.size() - 1))];
    r._75p = v[size_t(0.75 * (v.size() - 1))];
    r._90p = v[size_t(0.90 * (v.size() - 1))];
    r._95p = v[size_t(0.95 * (v.size() - 1))];
    r._99p = v[size_t(0.99 * (v.size() - 1))];

    double sum = 0.0;
    double sum_sq = 0.0;
    for (T e: v) {
      sum += e;
      sum_sq += (e * e);
    }
    double avg = sum / v.size();
    double sd = sqrt(sum_sq / v.size() - avg * avg);

    r.sum = sum;
    r.avg = avg;
    r.sd = sd;

    std::string stat = str(boost::format(
          "avg %s"
          "\nsd  %s"
          "\nmin %s"
          "\nmax %s"
          "\nsum %s"

          "\n 1p %s"
          "\n 5p %s"
          "\n10p %s"
          "\n25p %s"
          "\n50p %s"
          "\n75p %s"
          "\n90p %s"
          "\n95p %s"
          "\n99p %s")
        % r.avg % r.sd % r.min % r.max % r.sum
        % r._1p % r._5p % r._10p % r._25p % r._50p % r._75p % r._90p % r._95p % r._99p
        );
    //Cons::P(stat);

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

    return r;
  }
};
