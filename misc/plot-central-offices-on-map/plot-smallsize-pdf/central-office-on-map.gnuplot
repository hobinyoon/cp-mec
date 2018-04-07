#!/usr/bin/gnuplot
# Tested with gnuplot 4.6 patchlevel 6

IN_FNS = system("echo $IN_FNS")
IN_FN_SIZES = system("echo $IN_FN_SIZES")
DIST_SQ_THRESHOLDS = system("echo $DIST_SQ_THRESHOLDS")
OUT_FN = system("echo $OUT_FN")
USA_MAP = system("echo $HOME") . "/work/cp-mec/resource/usa-map-smallsize-0.05"

set print "-"

set terminal pdfcairo enhanced size 2.3in, (2.3*0.75)in
set output OUT_FN

do for [i=1:words(DIST_SQ_THRESHOLDS)] {
  reset

  d = word(DIST_SQ_THRESHOLDS, i)
  in_fn = word(IN_FNS, i)
  in_fn_size = word(IN_FN_SIZES, i)

  if (0) {
    set label sprintf("%s %s", d, in_fn_size) at screen 0.5,0.95 center
  }

  set xrange [-125:-66]
  set yrange [24:49.5]

  set lmargin screen 0
  set rmargin screen 1
  set tmargin screen 0
  set bmargin screen 1

  if (1) {
    set noborder
    set notics
  } else {
    set border front lc rgb "#808080" back
    set xtics nomirror tc rgb "black" autofreq -180,20
    set mxtics 2
    set ytics nomirror tc rgb "black" autofreq 0,10
    set mytics 2
    set grid xtics mxtics ytics mytics front lc rgb "#808080"
  }

  plot \
  in_fn u 2:1 w p pt 7 ps 0.07 lc rgb "red" not, \
  USA_MAP u 2:1 w filledcurves lw 1 fs transparent solid 0.0 border lc rgb "#404040" fc rgb "#FFFFFF" not
}

