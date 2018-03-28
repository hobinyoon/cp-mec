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
	//created_at = boost::posix_time::time_from_string(created_at_str);

	ifs.read((char*)&lat, sizeof(lat));
	ifs.read((char*)&lon, sizeof(lon));
	Util::ReadStr(ifs, obj_id);
	//ifs.read((char*)&youtube_video_uploader, sizeof(youtube_video_uploader));
	ifs.seekg(sizeof(long), ifs.cur);

	size_t topic_cnt;
	ifs.read((char*)&topic_cnt, sizeof(topic_cnt));
	for (size_t i = 0; i < topic_cnt; i ++) {
		//string t;
		//Util::ReadStr(ifs, t);
		//topics.push_back(t);
		size_t s;
		ifs.read((char*)&s, sizeof(s));
		ifs.seekg(s, ifs.cur);
	}
	ifs.read((char*)&type, sizeof(type));

	ifs.read((char*)&in_usa, sizeof(in_usa));
}

std::string OpYoutube::to_string() const {
	return str(
			//boost::format("%d %f %f %s %d %d %s %d %c")
			boost::format("%d %f %f %s %s %c")
			% type
			% lat
			% lon
			% obj_id
			//% id	// tweet id
			//% uid
			% created_at
			//% youtube_video_uploader
			% in_usa
			);
}


ostream& operator<< (ostream& os, const OpYoutube& op) {
	//os << boost::format("%ld %10ld %s %s %10lf %11lf %s %c")
	os << boost::format("%s %s %10lf %11lf %s %c")
		//% op.id
		//% op.uid
		% op.created_at
		% op.type
		% op.lat
		% op.lon
		% op.obj_id
		% op.in_usa;
	return os;
}
