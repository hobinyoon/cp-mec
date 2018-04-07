#!/usr/bin/gnuplot
#
# Tested with gnuplot 4.6 patchlevel 6

FN_IN = system("echo $FN_IN")
MAX_CLUSTER_SIZE = system("echo $MAX_CLUSTER_SIZE") + 0.0
FN_OUT = system("echo $FN_OUT")
US_STATES_MAP = "~/work/cp-mec/resource/usa-map-smallsize-0.05"

set print "-"
print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

set terminal pdfcairo enhanced size 2.3in, (2.3*0.65)in
set output FN_OUT

set noborder
set notics
if (0) {
  set xtics nomirror scale 0,0 font ",10" format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq -90,30,-30
  set ytics nomirror scale 0,0 font ",10" format "%.0f{/Symbol \260}" tc rgb "#808080" autofreq -90,15
}

set lmargin 0
set rmargin 0
set tmargin 0
set bmargin 0

if (1) {
  set xrange[-125:-66]
  set yrange [24:49.5]
} else {
  set xrange [-125:-66]
  set yrange [24:49.5]
}

color_bar_relative_value = 1

if (color_bar_relative_value) {
  set size 0.85, 1
} else {
  set size 0.78, 1
}

# Curve the color and size so that the big clusters are more noticeable.
color_1(s) = (s/MAX_CLUSTER_SIZE) ** 0.0001
#size_1(s) = s == 1 ? 0.05 : 0.30 * ((s/MAX_CLUSTER_SIZE) ** 0.05)
size_1(s) = 0.30 * ((s/MAX_CLUSTER_SIZE) ** 0.1)

num_colors=1000
set palette maxcolors num_colors
set palette model RGB defined (\
  0                  "#B0B0FF", \
  num_colors * 1 / 3 "green", \
  num_colors * 2 / 3 "yellow", \
  num_colors         "red")

if (color_bar_relative_value) {
  set cbtics ( \
    "1" color_1(MAX_CLUSTER_SIZE) \
    , "10^{-1}" color_1(MAX_CLUSTER_SIZE/10.0) \
    , "10^{-2}" color_1(MAX_CLUSTER_SIZE/100.0) \
    , "10^{-3}" color_1(MAX_CLUSTER_SIZE/1000.0) \
    , "10^{-4}" color_1(MAX_CLUSTER_SIZE/10000.0) \
    , "10^{-5}" color_1(MAX_CLUSTER_SIZE/100000.0) \
    , "10^{-6}" color_1(MAX_CLUSTER_SIZE/1000000.0) \
  ) font ",10" offset -0.5,0
  set colorbox user origin 0.87, 0.1 size 0.025, 0.85
} else {
  set cbtics ( \
    sprintf("%.0f", MAX_CLUSTER_SIZE) color_1(MAX_CLUSTER_SIZE) \
    ,                          "10^4" color_1(10000) \
    ,                          "10^3" color_1( 1000) \
    ,                          "10^2" color_1(  100) \
    ,                          "10^1" color_1(   10) \
    ,                             "1" color_1(    1) \
  )
  set colorbox user origin 0.82, 0.1 size 0.025, 0.85
}


plot \
FN_IN u 1:2:(size_1($3)):(color_1($3)) w p pt 7 ps variable palette not, \
US_STATES_MAP u 2:1 w filledcurves lw 1 fs transparent solid 0.0 border lc rgb "#808080" fc rgb "#FFFFFF" not

# "with points" makes about 1/5 smaller file size than "with circles"
