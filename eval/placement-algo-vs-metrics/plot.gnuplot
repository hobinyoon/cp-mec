#!/usr/bin/gnuplot
#
# Tested with gnuplot 4.6 patchlevel 6

FN_UB = "~/work/cp-mec/simulator/results/utility-based"
FN_UNI = "~/work/cp-mec/simulator/results/uniform"
FN_OUT = "placement-algo-vs-metrics.pdf"

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

size_x=2.8
#size_x=3.0
set terminal pdfcairo enhanced size (size_x)in, (size_x*0.75)in
set output FN_OUT

#set xtics format "10^{%T}" tc rgb "black"
#set ytics format "%.2f" tc rgb "black" #autofreq 0,0.01
set xtics tc rgb "black"
set ytics tc rgb "black" format "%.2f"
set grid xtics ytics
set border back lc rgb "#808080"

set xlabel "Total cache space allocated (TB)"
set ylabel "Aggregate\ncache hit ratio"

set yrange [0:]

set key bottom right

PS=0.3

plot \
FN_UNI u ($1/1024.0/1024):2 w lp pt 7 ps PS lc rgb "blue" t "Uniform", \
FN_UB  u ($1/1024.0/1024):2 w lp pt 7 ps PS lc rgb "red"  t "Utility-based, greedy"

# TODO: plot more points on the left
