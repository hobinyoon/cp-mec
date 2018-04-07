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
  fn_in = GetClusteredPoints()
  max_cluster_size = GetMaxClusterSize(fn_in)
  fn_out = "%s.pdf" % fn_in

  with Cons.MT("Plotting clustered locations ..."):
    env = os.environ.copy()
    env["FN_IN"] = fn_in
    env["MAX_CLUSTER_SIZE"] = str(max_cluster_size)
    env["FN_OUT"] = fn_out
    Util.RunSubp("gnuplot %s/geographic-dist-clustered.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


def GetClusteredPoints():
  dist_sq_threshold = Conf.Get("dist_sq_threshold")

  fn_in = Conf.GetFn("youtube_workload")
  fn_out = "%s/%s-clustered-with-dist-sq-%s" % (Conf.DnOut(), os.path.basename(fn_in), dist_sq_threshold)
  #Cons.P(fn_out)
  if os.path.isfile(fn_out):
    return fn_out
  cmd = "%s/_cluster.sh --youtube_workload=%s --dist_sq_threshold=%s" \
      % (os.path.dirname(__file__), fn_in, dist_sq_threshold)
  Util.RunSubp(cmd)
  return fn_out


def GetMaxClusterSize(fn):
  with open(fn) as fo:
    for line in fo:
      line = line.strip()
      if line.startswith("# max_cluster_size="):
        #Cons.P(line)
        t = line.split("=")
        if len(t) != 2:
          raise RuntimeError("Unexpected")
        return int(t[1])
  raise RuntimeError("Unexpected")


if __name__ == "__main__":
  sys.exit(main(sys.argv))
