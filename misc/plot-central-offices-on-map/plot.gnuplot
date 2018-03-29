#!/usr/bin/gnuplot
# Tested with gnuplot 4.6 patchlevel 6

OUT_FN = "central-offices-on-map.pdf"

set print "-"

set terminal pdfcairo enhanced size 8in, (8*0.8)in
set output OUT_FN

# TODO: play with removing practically duplicate points considering the output resolution.
# TODO: remove points that are not in the contiguous USA.

if (1) {
  IN_FN = "data"
  plot IN_FN u 2:1 w p pt 7 ps 0.05 not
}

if (0) {
  IN_FN = "/home/hobin/work/cp-mec/misc/calc-video-acc-reqs-at-central-offices/.output/co-accesses"
  plot IN_FN u 3:2 w p ps 0.5 not
}

print sprintf("Created %s", OUT_FN)
