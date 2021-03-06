#!/usr/bin/gnuplot
#
# Tested with gnuplot 4.6 patchlevel 6

FN_IN = system("echo $FN_IN")
FN_OUT = system("echo $FN_OUT")

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

size_x=2.3
#size_x=3.0
set terminal pdfcairo enhanced size (size_x)in, (size_x*0.75)in
set output FN_OUT

#set xtics format "%.0s*10^{%T}"
set xtics format "10^{%T}" tc rgb "black"
set ytics format "%.1f" tc rgb "black"
set grid xtics ytics
set border back lc rgb "#808080"

set xlabel "Number of accesses / CO"
set ylabel "CDF"

set logscale x

plot \
FN_IN u 1:2 w lp pt 7 ps 0.2 not

exit

#set lmargin 0
#set rmargin 0
#set tmargin 0
#set bmargin 0

#set xrange[-125:-66]
#set yrange [24:49.5]

#set noborder
#set notics

PS=0.2

LC="blue"

plot \
FN_IN u 4:3:(sqrt($2)/2) w p pt 7 ps variable lc rgb LC not, \
US_STATES_MAP u 2:1 w filledcurves lw 1 fs transparent solid 0.0 border lc rgb "#808080" fc rgb "#FFFFFF" not

# "with points" makes about 1/5 smaller file size than "with circles"
