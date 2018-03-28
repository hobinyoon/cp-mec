#!/usr/bin/gnuplot
# Tested with gnuplot 4.6 patchlevel 6

IN_FN = "data"
OUT_FN = "central-offices-on-map.pdf"

set print "-"

set terminal pdfcairo enhanced size 8in, (8*0.8)in
set output OUT_FN

# TODO: play with removing practically duplicate points considering the output resolution.
# TODO: remove points that are not in the contiguous USA.

plot IN_FN u 2:1 w p ps 0.001 not

print sprintf("Created %s", OUT_FN)
