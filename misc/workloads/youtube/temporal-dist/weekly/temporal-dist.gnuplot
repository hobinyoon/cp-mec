# Tested with gnuplot 4.6 patchlevel 6

FN_IN = system("echo $FN_IN")
WEEKLY_MAX = system("echo $WEEKLY_MAX") + 0
FN_OUT = system("echo $FN_OUT")

set print "-"
#print sprintf("MAX_CLUSTER_SIZE=%f", MAX_CLUSTER_SIZE)

set terminal pdfcairo enhanced size 2.3in, 1.6in

set output FN_OUT

set xtics scale 0,0 nomirror tc rgb "black" rotate by -45
set ytics nomirror tc rgb "black" autofreq 0,0.2 format "%.1f"
set grid xtics ytics back lc rgb "#808080"
set border back lc rgb "#808080"

#set xlabel "Day of week" offset 0,-0.4
set ylabel "Number of accesses\n(relative)" offset 0.5,0

set yrange[0:1]

set size 0.97,1

LW=2

if (1) {
  set xrange[-0.7:6.7]

  set boxwidth 0.7

  # Legend
  if (1) {
    x0=0.87
    y0=0.53
    x00=x0-0.01
    y00=y0-0.04
    x01=x0+0.1
    y01=y0+0.04
    set obj rect from screen x00,y00 to screen x01,y01 fs noborder front
    set label "Avg" at screen x0,y0 front

    y0=0.64
    y00=y0-0.04
    y01=y0+0.04
    set obj rect from screen x00,y00 to screen x01,y01 fs noborder front
    set label "Max" at screen x0,y0 front

    y0=0.32
    y00=y0-0.04
    y01=y0+0.04
    set obj rect from screen x00,y00 to screen x01,y01 fs noborder front
    set label "Min" at screen x0,y0 front
  }

  plot \
  FN_IN u 0:($2/WEEKLY_MAX):xtic(1) w boxes fs transparent solid 0.5 noborder fc rgb "red" not, \
  FN_IN u 0:($2/WEEKLY_MAX):($3/WEEKLY_MAX):($7/WEEKLY_MAX) w yerrorbars ps 0.0001 lw LW lc rgb "black" not, \

  #FN_IN u (x0):($2/WEEKLY_MAX):($0 == 6 ? "Avg" : "") w labels left tc rgb "black" not, \
  #FN_IN u (x0):($3/WEEKLY_MAX):($0 == 6 ? "Min" : "") w labels left tc rgb "black" not, \
  #FN_IN u (x0):($7/WEEKLY_MAX):($0 == 6 ? "Max" : "") w labels left tc rgb "black" not, \

  # yerrorbars
  # x  y  ylow  yhigh
}

if (0) {
  plot \
  FN_IN u 0:($4/WEEKLY_MAX):($3/WEEKLY_MAX):($7/WEEKLY_MAX):($6/WEEKLY_MAX) w candlesticks lw LW lc rgb "red" not whiskerbars, \
  FN_IN u 0:($5/WEEKLY_MAX):($5/WEEKLY_MAX):($5/WEEKLY_MAX):($5/WEEKLY_MAX) w candlesticks lw LW lc rgb "red" not, \
  FN_IN u 0:($2/WEEKLY_MAX):($2/WEEKLY_MAX):($2/WEEKLY_MAX):($2/WEEKLY_MAX):xtic(1) w candlesticks lw LW lc rgb "blue" not, \
  FN_IN u (-1):($2/WEEKLY_MAX):($0 == 0 ? "Avg" : "") w labels tc rgb "blue" not, \
  FN_IN u (-1):($3/WEEKLY_MAX):($0 == 0 ? "Min" : "") w labels tc rgb "red" not, \
  FN_IN u (-1):($4/WEEKLY_MAX):($0 == 0 ? "25%" : "") w labels tc rgb "red" not, \
  FN_IN u (-1):($5/WEEKLY_MAX):($0 == 0 ? "50%" : "") w labels tc rgb "red" not, \
  FN_IN u (-1):($6/WEEKLY_MAX):($0 == 0 ? "75%" : "") w labels tc rgb "red" not, \
  FN_IN u (-1):($7/WEEKLY_MAX):($0 == 0 ? "Max" : "") w labels tc rgb "red" not, \
# whisker plot: x  box_min  whisker_min  whisker_high  box_high
}

# Dow   avg  min  25p  50p  75p  max
# 1       2    3    4    5    6    7
