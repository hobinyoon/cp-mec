#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "conf.h"
#include "cons.h"
#include "op-youtube.h"
#include "util.h"
#include "youtube-data.h"

using namespace std;

namespace YoutubeData {
	// youtube
	vector<Op*> _entries;
	size_t _num_writes = 0;
	size_t _num_reads = 0;

	const vector<Op*>& Entries() {
		return _entries;
	}

	size_t NumReads() {
		return _num_reads;
	}

	size_t NumWrites() {
		return _num_writes;
	}

	void _LoadOps() {
		// Use boost::regex. C++11 regex works from 4.9. Ubuntu 14.04 has g++ 4.8.4.
		//   http://stackoverflow.com/questions/8060025/is-this-c11-regex-error-me-or-the-compiler
		string fn = boost::regex_replace(
				Conf::GetString("youtube_workload")
				, boost::regex("~")
				, Util::HomeDir());
		Cons::MT _(boost::format("Loading YouTube workload from file %s ...") % fn);

		ifstream ifs(fn.c_str(), ios::binary);
		if (! ifs.is_open())
			throw runtime_error(str(boost::format("unable to open file %1%") % fn));

		size_t e_size;
		ifs.read((char*)&e_size, sizeof(e_size));

		for (size_t i = 0; i < e_size; i ++) {
			Op* e = new OpYoutube(ifs);
			_entries.push_back(e);

			if (i % 10000 == 0) {
				Cons::ClearLine();
				Cons::Pnnl(boost::format("%.2f%%") % (100.0 * i / e_size));
			}
		}
		Cons::ClearLine();
		Cons::P("100.00%");

		Cons::P(boost::format("loaded %d ops.") % _entries.size());
	}

	void _TestInUsa() {
		// Make a file with coordinate, in_usa field for plotting
		Cons::MT _("Generating a (coordinate, in_usa) file ...");
		string dn = str(boost::format("%s/../.output") % boost::filesystem::path(__FILE__).parent_path().string());
		boost::filesystem::create_directories(dn);

		string fn = str(boost::format("%s/coord-in_usa-%s") % dn
				% boost::filesystem::path(Conf::GetString("youtube_workload")).filename().string());
		ofstream ofs(fn);
		for (auto e: _entries) {
			ofs << boost::format("%f %f\n")
				% e->lat
				% e->lon
        ;
		}
		ofs.close();
		Cons::P(boost::format("created file %s %d") % fn % boost::filesystem::file_size(fn));
		exit(0);
	}

	void Load() {
		_LoadOps();
		//_TestInUsa();
	}

	void FreeMem() {
		if (_entries.size() == 0)
			return;

		Cons::MT _("Freeing ops ...");
		for (auto e: _entries)
			delete e;
	}
}
