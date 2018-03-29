#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "tweet.h"
#include "util.h"

using namespace std;


Tweet::Tweet(long id_,
    long uid_,
    const string& created_at_,
    double geo_lati_,
    double geo_longi_,
    const string& youtube_video_id_)
: id(id_), uid(uid_), created_at(created_at_),
  geo_lati(geo_lati_), geo_longi(geo_longi_),
  youtube_video_id(youtube_video_id_)
{
}


Tweet::Tweet(ifstream& ifs) {
  ifs.read((char*)&id, sizeof(id));
  ifs.read((char*)&uid, sizeof(uid));
  Util::ReadStr(ifs, created_at);
  ifs.read((char*)&geo_lati, sizeof(geo_lati));
  ifs.read((char*)&geo_longi, sizeof(geo_longi));
  Util::ReadStr(ifs, youtube_video_id);
}


void Tweet::Write(ofstream& ofs) {
  ofs.write((char*)&id, sizeof(id));
  ofs.write((char*)&uid, sizeof(uid));
  Util::WriteStr(ofs, created_at);
  ofs.write((char*)&geo_lati, sizeof(geo_lati));
  ofs.write((char*)&geo_longi, sizeof(geo_longi));
  Util::WriteStr(ofs, youtube_video_id);
}


string to_string(const Tweet& t) {
  stringstream ss;
  ss << boost::format("%ld %10ld %s %10lf %11lf %s")
    % t.id
    % t.uid
    % t.created_at
    % t.geo_lati
    % t.geo_longi
    % t.youtube_video_id;
  return ss.str();
}


std::ostream& operator<< (std::ostream& os, const Tweet& t) {
  os << boost::format("%ld %10ld %s %10lf %11lf %s")
    % t.id
    % t.uid
    % t.created_at
    % t.geo_lati
    % t.geo_longi
    % t.youtube_video_id;
  return os;
}
