import argparse
import os
import sys

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons


_args = None

def ParseArgs():
	parser = argparse.ArgumentParser(
			description="Plot system resource usage"
			, formatter_class=argparse.ArgumentDefaultsHelpFormatter)

	parser.add_argument("--output_dir"
			, type=str
			, default=("%s/.output" % os.path.dirname(__file__))
			, help="Output directory")
	global _args
	_args = parser.parse_args()

	Cons.P("Parameters:")
	for a in vars(_args):
		Cons.P("%s: %s" % (a, getattr(_args, a)), ind=2)


def Get(k):
	return getattr(_args, k)

def GetDir(k):
	return Get(k).replace("~", os.path.expanduser("~"))
