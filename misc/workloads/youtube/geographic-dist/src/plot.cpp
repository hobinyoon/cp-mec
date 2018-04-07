#include <boost/filesystem.hpp>

#include "clusterer.h"
#include "conf.h"
#include "cons.h"
#include "plot.h"
#include "util.h"

using namespace std;

namespace Plot {
	void Run() {
//		if (! boost::filesystem::exists(Conf::fn_plot_data))
//			Clusterer::Run();
//
//		Cons::MT _("Plotting ...");
//
//		string fn_out = Conf::fn_plot_data + ".pdf";
//		Util::SetEnv("FN_IN", Conf::fn_plot_data);
//		Util::SetEnv("MAX_CLUSTER_SIZE", to_string(Clusterer::MaxClusterSize()));
//		Util::SetEnv("FN_OUT", fn_out);
//		string cmd = str(boost::format("gnuplot %s/youtube-access-locations-clustered.gnuplot")
//				% boost::filesystem::path(__FILE__).parent_path().string());
//		Cons::P(Util::RunSubprocess(cmd));
//		Cons::P(boost::format("Created %s %d") % fn_out % boost::filesystem::file_size(fn_out));
	}
};
