#!/usr/bin/gnuplot
#   Tested with gnuplot 4.6 patchlevel 6

#FN_RAW = "data-raw"
#FN_CV = "data-convex"
FN_IN = "data"
FN_OUT = "utilitycurves-raw-vs-convex.pdf"

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

size_x=3.5
set terminal pdfcairo enhanced size (size_x)in, (size_x*0.55)in
set output FN_OUT

set key bottom right #opaque

set logscale x

set xtics tc rgb "black" ( \
  "1MB"              1, \
  "1GB"           1024, \
  "1TB"      1024*1024, \
  "10"    10*1024*1024, \
  "100"  100*1024*1024, \
  "1PB" 1024*1024*1024 \
)

set ytics nomirror tc rgb "black" format "%.2f"
set y2tics tc rgb "black" format "%.2f"

set grid xtics ytics lc rgb "#808080" back
set border back lc rgb "#808080" back

set xlabel "Total cache space allocated"
set ylabel "Aggregate cache hit ratio"
set y2label "Gain of convex curves\nover unmodified curves" tc rgb "red"

set xrange [1024:]
set y2range [0:]

PS=0.9

set boxwidth 0.7 relative

plot \
FN_IN u 1:(($2-$5)/$5) axes x1y2 w boxes fs transparent solid 0.35 noborder lc rgb "red" not, \
FN_IN u 1:2 w lp pt 1 ps PS lc rgb "red"  t "Convex", \
FN_IN u 1:5 w lp pt 2 ps PS lc rgb "blue" t "Unmodified", \
