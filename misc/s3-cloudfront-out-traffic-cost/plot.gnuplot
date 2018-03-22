#!/usr/bin/gnuplot
# Tested with gnuplot 4.6 patchlevel 6

IN_FN_CF = "data-cloudfront"
IN_FN_S3 = "data-s3"
OUT_FN = "s3-cloudfront-out-traffic-cost.pdf"

set print "-"
#print sprintf("NUM_STGDEVS=%d", NUM_STGDEVS)

set terminal pdfcairo enhanced size 2.3in, (2.3*0.8)in
set output OUT_FN

set xtics nomirror tc rgb "black" ( \
"0"    0.1/1024.0/1024 , \
"1\nGB" 1/1024.0/1024 , \
"1\nTB" 1/1024.0 , \
"1\nPB" 1 \
)
set ytics nomirror tc rgb "black" autofreq 0,0.02 format "%0.2f"
set mytics 2
set grid xtics ytics mytics back lc rgb "#808080"
set border back lc rgb "#808080" back

set xlabel "Data transferred (/month)" offset 0,-1
set ylabel "Cost ($/GB)"

#set xrange [0:7]
set xrange [0.1/1024/1024:50]
set yrange [0:]

set logscale x

C_CF = "red"
C_S3 = "blue"

# Labels
if (1) {
  x0 = 0.9
  y0 = 0.61
  x00 = x0 - 0.01
  x01 = x0 + 0.1
  y00 = y0 - 0.03
  y01 = y0 + 0.04
  set obj rect from screen x00,y00 to screen x01,y01 fs noborder fc rgb "white" front
  set label "S3" at screen x0,y0 tc rgb C_S3 front

  x0 = 0.9
  y0 = 0.42
  x00 = x0 - 0.01
  x01 = x0 + 0.1
  y00 = y0 - 0.03
  y01 = y0 + 0.04
  set obj rect from screen x00,y00 to screen x01,y01 fs noborder fc rgb "white" front
  set label "CF" at screen x0,y0 tc rgb C_CF front
}

plot \
IN_FN_CF u ($1/1024):3 w steps lc rgb C_CF lw 2 not, \
IN_FN_S3 u ($1/1024):3 w steps lc rgb C_S3 lw 2 not
# x y xdelta ydelta

#IN_FN_CF u 1:3 w fillsteps fs solid fc rgb "red"
#IN_FN_CF u 1:3:($2-$1):(0) w vectors nohead lw 4 not, \

print sprintf("Created %s", OUT_FN)

exit





# Get ranges
if (1) {
  set terminal unknown
  set xdata time
  set timefmt "%H:%M:%S"

  plot IN_FN_YCSB u 1:2 w l
  #Y_MAX_DB_IOPS=GPVAL_DATA_Y_MAX
  Y_MIN_DB_IOPS=GPVAL_Y_MIN
  Y_MAX_DB_IOPS=GPVAL_Y_MAX

  #show variables all
}



LMARGIN = 0.17
set sample 1000

# Hide the SSTable loading phase. Not to distract the reviewers
#TIME_MIN = "00:00:00"
TIME_MIN = "00:00:12"

# autofreq interval
AFI = 15 * 60

# Cost change labels
if (1) {
  reset
  set xdata time
  set timefmt "%H:%M:%S"

  set notics
  set noborder

  # Align the stacked plots
  set lmargin screen LMARGIN
  set bmargin screen 0.05

  Y_MAX=1
  set xrange [TIME_MIN:TIME_MAX]
  set yrange [0:Y_MAX]

  LW = 4

  y_t = 0.09
  y_b = 0.0
  y_m = (y_b + y_t) / 2.0

  y_t1 = y_t + 0.10
  y_t2 = y_t1 + 0.15
  y_t3 = y_t2 + 0.15

  set label "Target cost ($/GB/month)" at TIME_MIN, y_t3 offset -1.5, 0 tc rgb "black"
  set label "Initial value" at TIME_MIN, y_t2 offset -1.5, 0 tc rgb "black"
  set label "Changes" at TIME_MIN, y_t2 offset  8.9, 0 tc rgb "black"

  #set arrow from TIME_MIN, y_m to TIME_MAX, y_m nohead lc rgb "black" lw LW front
  do for [i=1:words(TARGET_COST_CHANGES_TIME)] {
    if (i == 1) {
      x0 = TIME_MIN
    } else {
      x0 = word(TARGET_COST_CHANGES_TIME, i)
    }
    set arrow from x0, y_t to x0, y_b head lc rgb "black" lw LW front
    set label word(TARGET_COST_CHANGES_COST, i) at x0, y_t1 center tc rgb "black"
  }

  plot x w l lc rgb "white" not
}
