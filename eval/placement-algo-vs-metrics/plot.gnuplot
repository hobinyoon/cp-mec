#!/usr/bin/gnuplot
#
# Tested with gnuplot 4.6 patchlevel 6

FN_UNI = "data-uniform"
FN_RVB = "data-reqvolbased"
FN_UB = "data-utilcurvebased"
FN_OUT = "placement-algo-vs-metrics.pdf"

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

size_x=2.8
#size_x=3.0
set terminal pdfcairo enhanced size (size_x)in, (size_x*0.75)in
set output FN_OUT

#set xtics format "10^{%T}" tc rgb "black"
#set ytics format "%.2f" tc rgb "black" #autofreq 0,0.01
set xtics nomirror tc rgb "black"
set ytics nomirror tc rgb "black" format "%.2f"
set grid xtics ytics
set border back lc rgb "#808080"

set xlabel "Total cache space allocated (TB)"
set ylabel "Aggregate cache hit ratio"

set xrange [0:]
set yrange [0:]

set key bottom right

LW=2

plot \
FN_UB  u ($1/1024.0/1024):2 w lp lw LW lc rgb "red"     t "UC", \
FN_RVB u ($1/1024.0/1024):2 w lp lw LW lc rgb "blue"    t "REQ", \
FN_UNI u ($1/1024.0/1024):2 w lp lw LW lc rgb "#006400" t "UNI", \

# "Utility curve based"
# "Request volume based"
# "Uniform"
