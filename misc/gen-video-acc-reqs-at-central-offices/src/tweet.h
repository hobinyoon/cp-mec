#pragma once

#include <string>
#include <vector>

struct Tweet {
  long id;
  long uid;
  std::string created_at;
  double geo_lati;
  double geo_longi;
  std::string youtube_video_id;

  Tweet(long id_,
      long uid_,
      const std::string& created_at_,
      double geo_lati_,
      double geo_longi_,
      const std::string& youtube_video_id_);

  Tweet(std::ifstream& ifs);

  void Write(std::ofstream& ofs);
};

std::string to_string(const Tweet& t);
std::ostream& operator<< (std::ostream& os, const Tweet& op);
