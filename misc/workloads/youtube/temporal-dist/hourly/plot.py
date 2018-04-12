#!/usr/bin/env python

import os
import re
import sys

sys.path.insert(0, "%s/work/cp-mec/util" % os.path.expanduser("~"))
import Cons
import Util

import Conf


def main(argv):
  Conf.Init()
  Plot()


def Plot():
  (fn_in, hourly_max) = GetTemporalDist()
  fn_out = "%s.pdf" % fn_in

  with Cons.MT("Plotting temporal distribution ..."):
    env = os.environ.copy()
    env["FN_IN"] = fn_in
    env["HOURLY_MAX"] = hourly_max
    env["FN_OUT"] = fn_out
    Util.RunSubp("gnuplot %s/temporal-dist.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


def GetTemporalDist():
  fn_in = Conf.GetFn("youtube_workload")
  fn_out = "%s/%s-temporal-dist-hourly" % (Conf.DnOut(), os.path.basename(fn_in))
  if os.path.isfile(fn_out):
    return (fn_out, _GetHourlyMax(fn_out))
  cmd = "%s/_gen-plot-data.sh --youtube_workload=%s --out_fn=%s" % (os.path.dirname(__file__), fn_in, fn_out)
  Util.RunSubp(cmd)
  return (fn_out, _GetHourlyMax(fn_out))


def _GetHourlyMax(fn):
  with open(fn) as fo:
    for line in fo:
      if line.startswith("# hourly_max="):
        mo = re.match(r"# hourly_max=(?P<v>(\d)+)", line)
        if mo is None:
          raise RuntimeError("Unexpected")
        return mo.group("v")


if __name__ == "__main__":
  sys.exit(main(sys.argv))
