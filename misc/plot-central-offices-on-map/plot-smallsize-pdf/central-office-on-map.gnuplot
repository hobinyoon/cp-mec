#!/usr/bin/gnuplot
# Tested with gnuplot 4.6 patchlevel 6

IN_FNS = system("echo $IN_FNS")
IN_FN_SIZES = system("echo $IN_FN_SIZES")
DIST_SQ_THRESHOLDS = system("echo $DIST_SQ_THRESHOLDS")
OUT_FN = system("echo $OUT_FN")

set print "-"

set terminal pdfcairo enhanced size 2.3in, (2.3*0.8)in
set output OUT_FN

do for [i=1:words(DIST_SQ_THRESHOLDS)] {
  reset

  d = word(DIST_SQ_THRESHOLDS, i)
  in_fn = word(IN_FNS, i)
  in_fn_size = word(IN_FN_SIZES, i)

  if (0) {
    set label sprintf("%s %s", d, in_fn_size) at screen 0.5,0.95 center
  }

  set xrange [-128:-65]
  set yrange [23:50]

  set border front lc rgb "#808080" back
  set xtics nomirror tc rgb "black" autofreq -180,20
  set mxtics 2
  set ytics nomirror tc rgb "black" autofreq 0,10
  set mytics 2
  set grid xtics mxtics ytics mytics front lc rgb "#808080"

  plot in_fn u 2:1 w p pt 7 ps 0.07 not
}

