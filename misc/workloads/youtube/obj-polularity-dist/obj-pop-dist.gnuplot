# Tested with gnuplot 4.6 patchlevel 6

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

if (1) {
  set terminal unknown
  plot FN_IN u 1:2 w p
  Y_MAX=GPVAL_DATA_Y_MAX
}

set terminal pdfcairo enhanced size 2.3in, (2.3*0.85)in
set output FN_OUT

set rmargin screen 0.97

set xtics nomirror tc rgb "black" ( \
  "1"          1 \
, "10"        10 \
, "10^2"     100 \
, "10^3"    1000 \
, "10^4"   10000 \
, "10^5"  100000 \
, "10^6" 1000000 \
)

set ytics nomirror tc rgb "black" ( \
  "1"       1 \
, "10^{-1}" 0.1 \
, "10^{-2}" 0.01 \
, "10^{-3}" 0.001 \
)

set grid xtics ytics front lc rgb "#808080"
set border back lc rgb "#808080"

set xlabel "Object rank"
set ylabel "Number of accesses\n(relative)"

set logscale xy

plot \
FN_IN u 1:($2/Y_MAX) w l lw 2 not
