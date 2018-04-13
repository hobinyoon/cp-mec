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
  fn_in = "data-cloudfront-locs"
  fn_out = "%s/cloudfront-locs.pdf" % Conf.DnOut()

  with Cons.MT("Plotting CF locations ..."):
    env = os.environ.copy()
    env["FN_IN"] = fn_in
    env["FN_OUT"] = fn_out
    Util.RunSubp("gnuplot %s/cf-locs.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


if __name__ == "__main__":
  sys.exit(main(sys.argv))
