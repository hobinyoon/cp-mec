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
set xrange [0.1/1024/1024:100]
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
IN_FN_S3 u ($1/1024):3 w steps lc rgb C_S3 lw 2 not, \
IN_FN_CF u ($1/1024):3 w steps lc rgb C_CF lw 2 not
# x y xdelta ydelta

#IN_FN_CF u 1:3 w fillsteps fs solid fc rgb "red"
#IN_FN_CF u 1:3:($2-$1):(0) w vectors nohead lw 4 not, \

print sprintf("Created %s", OUT_FN)
