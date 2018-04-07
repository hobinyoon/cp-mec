#!/usr/bin/env python

import os
import sys

sys.path.insert(0, "%s/work/cp-mec/util" % os.path.expanduser("~"))
import Cons
import Util

import Conf


def main(argv):
  Conf.Init()
  Plot()


def Plot():
  fn_in = GetObjPopDist()
  fn_out = "%s.pdf" % fn_in

  with Cons.MT("Plotting obj pop distribution ..."):
    env = os.environ.copy()
    env["FN_IN"] = fn_in
    env["FN_OUT"] = fn_out
    Util.RunSubp("gnuplot %s/obj-pop-dist.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


def GetObjPopDist():
  fn_in = Conf.GetFn("youtube_workload")
  fn_out = "%s/%s-obj-pop-dist" % (Conf.DnOut(), os.path.basename(fn_in))
  if os.path.isfile(fn_out):
    return fn_out

  cmd = "%s/_gen-plot-data.sh --youtube_workload=%s" % (os.path.dirname(__file__), fn_in)
  Util.RunSubp(cmd)
  return fn_out


if __name__ == "__main__":
  sys.exit(main(sys.argv))
