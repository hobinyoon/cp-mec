#!/usr/bin/env python

import os
import sys

sys.path.insert(0, "%s/work/cp-mec/util" % os.path.expanduser("~"))
import Cons
import Util

import Conf
import Stat


def main(argv):
  Conf.Init()
  Plot()


def Plot():
  fn_in = GetNumAccessesStat()
  fn_out = "%s.pdf" % fn_in

  with Cons.MT("Plotting ..."):
    env = os.environ.copy()
    env["FN_IN"] = fn_in
    env["FN_OUT"] = fn_out
    Util.RunSubp("gnuplot %s/cdf.gnuplot" % os.path.dirname(__file__), env=env)
    Cons.P("Created %s %d" % (fn_out, os.path.getsize(fn_out)))


def GetNumAccessesStat():
  fn_out = "%s/cdf-youtube-accesses-per-co" % Conf.DnOut()
  if os.path.exists(fn_out):
    return fn_out

  num_accesses = []
  fn_in = Conf.GetFn("video_accesses_by_COs")
  with open(fn_in) as fo:
    while True:
      line = fo.readline()
      if len(line) == 0:
        break

      line = line.strip()
      if len(line) == 0:
        continue
      if line[0] == "#":
        continue

      # 4 34.3305 -111.091 13
      t = line.split(" ")
      if len(t) != 4:
        raise RuntimeError("Unexpected: [%s]" % line)
      n = int(t[3])
      #Cons.P(n)
      num_accesses.append(n)

      for j in range(n):
        if len(fo.readline()) == 0:
          raise RuntimeError("Unexpected")

  r = Stat.Gen(num_accesses, fn_out)
  #Cons.P(r)

  return fn_out


if __name__ == "__main__":
  sys.exit(main(sys.argv))
