#!/usr/bin/gnuplot
#
# Tested with gnuplot 4.6 patchlevel 6

FN_UNI = "data-uniform"
FN_RVB = "data-reqvolbased"
FN_USR = "data-userbased"
FN_UB = "data-utilcurvebased"
FN_UB_RVB = "data-utilcurvebased-reqvolbased"
FN_OUT = "placement-algo-vs-metrics.pdf"

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

LMARGIN=0.23
RMARGIN=0.98

if (1) {
  logscale_x = 1

  size_x=2.8
  #size_x=3.0
  set terminal pdfcairo enhanced size (size_x)in, (size_x*0.75)in
  set output FN_OUT

  set lmargin screen LMARGIN
  set rmargin screen RMARGIN

  #set xtics format "10^{%T}" tc rgb "black"
  #set ytics format "%.2f" tc rgb "black" #autofreq 0,0.01

  if (logscale_x) {
    set xtics nomirror tc rgb "black" ( \
      "1GB"         1024, \
      "4"         4*1024, \
      "16"       16*1024, \
      "64"       64*1024, \
      "256"     256*1024, \
      "1TB"    1024*1024, \
      "4"    4*1024*1024, \
      "16"  16*1024*1024 \
    )
    set xlabel "Total cache space allocated"
    set key top left
  } else {
    set xtics nomirror tc rgb "black"
    set xlabel "Total cache space allocated (TB)"
    set key bottom right
  }

  set ytics nomirror tc rgb "black" format "%.2f"
  set grid xtics ytics
  set border back lc rgb "#808080"

  set ylabel "Aggregate cache hit ratio"

  set yrange [0:]

  LW=2

  if (logscale_x) {
    set logscale x
    set xrange [1024:32*1024*1024]
    plot \
    FN_UB  u 1:2 w lp lw LW lc rgb "red"     t "UC", \
    FN_RVB u 1:2 w lp lw LW lc rgb "blue"    t "REQ", \
    FN_USR u 1:2 w lp lw LW lc rgb "#006400" t "USR", \
    FN_UNI u 1:2 w lp lw LW lc rgb "#8A2BE2" t "UNI"
  } else {
    set xrange [0:]

    plot \
    FN_UB  u ($1/1024.0/1024):2 w lp lw LW lc rgb "red"     t "UC", \
    FN_RVB u ($1/1024.0/1024):2 w lp lw LW lc rgb "blue"    t "REQ", \
    FN_USR u ($1/1024.0/1024):2 w lp lw LW lc rgb "#006400" t "USR", \
    FN_UNI u ($1/1024.0/1024):2 w lp lw LW lc rgb "#8A2BE2" t "UNI"
  }
}


if (1) {
  reset
  set lmargin screen LMARGIN
  set rmargin screen RMARGIN

  set logscale xy
  set xrange [1024:32*1024*1024]
  #set yrange [0:0.025]

  #set ylabel "Improvement of UC-based cache space allocation\nover request volume-based allocation"
  set ylabel "Improvement of UC\nover REQ (%)" offset -0.5,0
  set xlabel "Total cache space allocated"

  set xtics nomirror tc rgb "black" ( \
      "1GB"         1024, \
      "4"         4*1024, \
      "16"       16*1024, \
      "64"       64*1024, \
      "256"     256*1024, \
      "1TB"    1024*1024, \
      "4"    4*1024*1024, \
      "16"  16*1024*1024 \
      )
  #set ytics nomirror tc rgb "black" #format "%.2f"
  set ytics nomirror tc rgb "black" format "10^{%T}"
  set grid xtics ytics
  set border back lc rgb "#808080"

  set boxwidth 0.7 relative

  plot \
  FN_UB_RVB u 1:(100.0*($3-$6)/$6) w boxes fs transparent solid 0.5 noborder not

  #FN_UB_RVB u (1024*1024 < $1 ? $1 : 1/0):(($3-$6)/$6) w boxes fs transparent solid 0.7 noborder not
}


if (1) {
  reset
  set lmargin screen LMARGIN
  set rmargin screen RMARGIN

  set logscale x
  set xrange [1024:32*1024*1024]
  set yrange [0:0.025]

  set ylabel "Improvement of UC over REQ\n(aggregate hit rate)" offset 1,0
  set xlabel "Total cache space allocated"

  set xtics nomirror tc rgb "black" ( \
      "1GB"         1024, \
      "4"         4*1024, \
      "16"       16*1024, \
      "64"       64*1024, \
      "256"     256*1024, \
      "1TB"    1024*1024, \
      "4"    4*1024*1024, \
      "16"  16*1024*1024 \
      )
  set ytics nomirror tc rgb "black" format "%.2f" autofreq 0,0.01
  set mytics 2
  set grid xtics ytics mytics
  set border back lc rgb "#808080"

  set boxwidth 0.7 relative

  plot \
  FN_UB_RVB u 1:($2-$5) w boxes fs transparent solid 0.5 noborder not
}
