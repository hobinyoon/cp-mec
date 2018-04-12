# Tested with gnuplot 4.6 patchlevel 6

FN_IN = system("echo $FN_IN")
HOURLY_MAX = system("echo $HOURLY_MAX") + 0
FN_OUT = system("echo $FN_OUT")

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

size_x=4.0
set terminal pdfcairo enhanced size (size_x)in, (size_x*0.4)in
set output FN_OUT

set xtics scale 0,0 nomirror tc rgb "black" autofreq 0,2
set ytics nomirror tc rgb "black" autofreq 0,0.2 format "%.1f"
set grid xtics ytics back lc rgb "#808080"
set border back lc rgb "#808080"

#set xlabel "Day of week" offset 0,-0.4
set ylabel "Number of accesses\n(relative)" offset 0.5,0

set yrange[0:1]
#set xrange[-0.3:24.3]
set xrange[0:24]

set size 0.97,1

# Legend
if (1) {
  x0=0.92
  y0=0.39
  x00=x0-0.005
  y00=y0-0.04
  x01=x0+0.1
  y01=y0+0.04
  set obj rect from screen x00,y00 to screen x01,y01 fs noborder front
  set label "Avg" at screen x0,y0 front

  y0=0.61
  y00=y0-0.04
  y01=y0+0.04
  set obj rect from screen x00,y00 to screen x01,y01 fs noborder front
  set label "Max" at screen x0,y0 front

  y0=0.19
  y00=y0-0.04
  y01=y0+0.04
  set obj rect from screen x00,y00 to screen x01,y01 fs noborder front
  set label "Min" at screen x0,y0 front
}

# x width / 2
xw2=0.35

LW=2
set bars 0.6

plot \
FN_IN u ($1+0.5-xw2):(0):($1+0.5-xw2):($1+0.5+xw2):(0):($2/HOURLY_MAX) w boxxyerrorbars fs transparent solid 0.5 noborder fc rgb "red" not, \
FN_IN u ($1+0.5):($2/HOURLY_MAX):($3/HOURLY_MAX):($7/HOURLY_MAX) w yerrorbars ps 0.0001 lw LW lc rgb "black" not, \

# boxxyerrorbars
# x  y  xlow  xhigh  ylow  yhigh


#FN_IN u 1:($2/HOURLY_MAX) w boxes fs transparent solid 0.5 noborder fc rgb "red" not, \

# yerrorbars
# x  y  ylow  yhigh

