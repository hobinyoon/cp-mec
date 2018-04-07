#include <fstream>
#include <boost/format.hpp>
#include "op-youtube.h"
#include "util.h"

using namespace std;


OpYoutube::OpYoutube(ifstream& ifs) {
	//ifs.read((char*)&id, sizeof(id));
	ifs.seekg(sizeof(long), ifs.cur);

	//ifs.read((char*)&uid, sizeof(uid));
	ifs.seekg(sizeof(long), ifs.cur);

	Util::ReadStr(ifs, created_at);
  // TODO: think about if you need the conversion
	//created_at = boost::posix_time::time_from_string(created_at_str);

	//ifs.read((char*)&lat, sizeof(lat));
	//ifs.read((char*)&lon, sizeof(lon));
	ifs.seekg(sizeof(lat) * 2, ifs.cur);

	Util::ReadStr(ifs, obj_id);
}

std::string OpYoutube::to_string() const {
	return str(
			//boost::format("%f %f %s %d %d %s %d")
			boost::format("%f %f %s %s")
			% lat
			% lon
			% obj_id
			//% id	// tweet id
			//% uid
			% created_at
			//% youtube_video_uploader
			);
}


ostream& operator<< (ostream& os, const OpYoutube& op) {
	//os << boost::format("%ld %10ld %s %s %10lf %11lf %s")
	os << boost::format("%s %10lf %11lf %s")
		//% op.id
		//% op.uid
		% op.created_at
		% op.lat
		% op.lon
		% op.obj_id
    ;
	return os;
}
