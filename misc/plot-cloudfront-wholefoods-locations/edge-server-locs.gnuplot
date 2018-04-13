#!/usr/bin/gnuplot
#
# Tested with gnuplot 4.6 patchlevel 6

FN_CF = system("echo $FN_CF")
FN_WF = system("echo $FN_WF")
FN_OUT = system("echo $FN_OUT")
US_STATES_MAP = "~/work/cp-mec/resource/usa-map-smallsize-0.05"

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

#size_x=2.3
size_x=3.0
#set terminal pdfcairo enhanced size (size_x)in, (size_x*0.75)in
set terminal pdfcairo enhanced size (size_x)in, (size_x*0.80)in
set output FN_OUT

set lmargin 0
set rmargin 0
set tmargin 0
set bmargin 0

set xrange[-125:-66]
#set yrange [24:49.5]
set yrange [22:49.5]

set noborder
set notics

PS=0.2

C_CF="blue"
C_WF="red"

TP_CF=0.7
TP_WF=0.6

# CloudFront circle size
s0(a)=sqrt(a)*0.5

# Legend
if (1) {
  x0=0.02
  y0=0.05
  set label "CloudFront:" at screen x0,y0

  x0=x0+0.28
  c_size=4
  set label sprintf("%d", c_size) at screen x0,y0 right offset -1,0
  set obj circle at screen x0,y0 size s0(c_size) fs solid TP_CF noborder fc rgb C_CF

  x0=x0+0.1
  c_size=c_size-1
  set label sprintf("%d", c_size) at screen x0,y0 right offset -1,0
  set obj circle at screen x0,y0 size s0(c_size) fs solid TP_CF noborder fc rgb C_CF

  x0=x0+0.1
  c_size=c_size-1
  set label sprintf("%d", c_size) at screen x0,y0 right offset -1,0
  set obj circle at screen x0,y0 size s0(c_size) fs solid TP_CF noborder fc rgb C_CF

  x0=x0+0.1
  c_size=c_size-1
  set label sprintf("%d", c_size) at screen x0,y0 right offset -1,0
  set obj circle at screen x0,y0 size s0(c_size) fs solid TP_CF noborder fc rgb C_CF

  x0=x0+0.08
  set label "Whole Foods:" at screen x0,y0

  x0=x0+0.27
  set obj circle at screen x0,y0 size s0(c_size) fs solid TP_WF noborder fc rgb C_WF
}

plot \
FN_CF u 4:3:(s0($2)) w circles fs transparent solid TP_CF noborder fc rgb C_CF not, \
FN_WF u 3:2:(0.3) w circles fs transparent solid TP_WF noborder fc rgb C_WF not, \
US_STATES_MAP u 2:1 w filledcurves lw 1 fs transparent solid 0.0 border lc rgb "#808080" fc rgb "#FFFFFF" not

#FN_CF u 4:3:(sqrt($2)*0.4) w p pt 7 ps variable lc rgb C_CF not, \
#FN_WF u 3:2 w p pt 7 ps PS lc rgb C_WF not, \

# "with points" makes about 1/5 smaller file size than "with circles"
