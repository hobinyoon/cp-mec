#!/usr/bin/env python

import os
import re
import sys

sys.path.insert(0, "%s/work/mutant/ec2-tools/lib/util" % os.path.expanduser("~"))
import Cons
import Util

import Conf


def main(argv):
  Conf.ParseArgs()
  Util.MkDirs(Conf.GetDir("output_dir"))

  dist_sq_thresholds = [
      0
      , 0.008
      , 0.01
      , 0.02
      , 0.05]

  #dist_sq_thresholds = [0.02]

  dist_sq_thresholds_str = []
  for d in dist_sq_thresholds:
    dist_sq_thresholds_str.append(_NoTrailing0s(d))

  reduced_files = []
  reduced_file_sizes = []
  with Cons.MT("Generating reduced size usa map ..."):
    for d in dist_sq_thresholds_str:
      fn_co_loc = "filter-out-almost-duplicate-points/.output/usa-map-smallsize-%s" % d
      reduced_files.append(fn_co_loc)
      if not os.path.exists(fn_co_loc):
        cmd = "cd filter-out-almost-duplicate-points && ./build-and-run.sh --dist_sq_threshold=%s" % d
        Util.RunSubp(cmd)
      reduced_file_sizes.append(os.path.getsize(fn_co_loc))

  dn_out = "%s/.output" % os.path.dirname(__file__)
  fn_out = "%s/usa-map.pdf" % dn_out
  with Cons.MT("Plotting ..."):
    env = os.environ.copy()
    env["IN_FNS"] = " ".join(reduced_files)
    env["IN_FN_SIZES"] = " ".join(str(s) for s in reduced_file_sizes)
    env["DIST_SQ_THRESHOLDS"] = " ".join(dist_sq_thresholds_str)
    env["OUT_FN"] = fn_out
    Util.RunSubp("gnuplot %s/usa-map.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


def _NoTrailing0s(d):
  d_str = "%f" % d
  while True:
    mo = re.match(r".+\.\d+0+$", d_str)
    if mo is None:
      break
    d_str = d_str[:-1]
  return d_str


if __name__ == "__main__":
  sys.exit(main(sys.argv))
