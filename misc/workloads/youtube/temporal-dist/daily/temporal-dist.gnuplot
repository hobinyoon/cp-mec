# Tested with gnuplot 4.6 patchlevel 6

FN_IN = system("echo $FN_IN")
DAILY_MAX = system("echo $DAILY_MAX") + 0
FN_OUT = system("echo $FN_OUT")

set print "-"
print sprintf("DAILY_MAX=%.0f", DAILY_MAX)

wide_plot = 1
if (wide_plot) {
  set terminal pdfcairo enhanced size 4.3in, (4.3*0.40)in
} else {
  set terminal pdfcairo enhanced size 2.3in, (2.3*0.85)in
}
set output FN_OUT

set rmargin screen 0.96

set xdata time
set timefmt "%Y-%m-%d"

if (wide_plot) {
  set xtics nomirror tc rgb "black" ( \
    "Jun" "2016-06-01" \
  , ""    "2016-07-01" \
  , "Aug" "2016-08-01" \
  , ""    "2016-09-01" \
  , "Oct" "2016-10-01" \
  , ""    "2016-11-01" \
  , "Dec" "2016-12-01" \
  , ""    "2017-01-01" \
  , "Feb" "2017-02-01" \
  , ""    "2017-03-01" \
  , "Apr" "2017-04-01" \
  , ""    "2017-05-01" \
  , "Jun" "2017-05-31" \
  )
} else {
  set xtics nomirror tc rgb "black" ( \
    "Jun" "2016-06-01" \
  , "Aug" "2016-08-01" \
  , "Oct" "2016-10-01" \
  , "Dec" "2016-12-01" \
  , "Feb" "2017-02-01" \
  , "Apr" "2017-04-01" \
  , "Jun" "2017-05-31" \
  )
}

# mxtics can not be set when xtics intervals are irregular or are explicitly set

set grid xtics ytics front lc rgb "#808080"
set border back lc rgb "#808080"

set xlabel "Time (month)"

set xrange ["2016-06-01":"2017-05-31"]

relative_y = 1
if (relative_y) {
  set yrange [0:]
  set ytics nomirror tc rgb "black" format "%.1f"
  set ylabel "Number of accesses\n(relative)" offset 0.5,0
  plot FN_IN u 1:($2/DAILY_MAX) w lp pt 7 ps 0.1 not
} else {
  set yrange [0:]
  set ytics nomirror tc rgb "black" autofreq 0,1
  set ylabel "Number of accesses (K)" offset 1,0
  plot FN_IN u 1:($2/1000) w lp pt 7 ps 0.1 not
}

# Hard to explain
#FN_IN u 1:($2/1000) w l smooth bezier lw 2 lc "blue" not

# Dips on New year. Can you generalize this to holidays? Including christmas, thanks giving.

# Attemped histogram. Wasn't very pretty.
if (0) {
  _prev = 0
  prev0(a) = _prev
  prev1(a) = (_prev = a)

  plot \
  FN_IN u 1:(0):1:(timecolumn(1) + 24 * 3660):(0):($2/1000) w boxxyerrorbars fill solid 0.3 noborder not, \

  #FN_IN u 1:(0):1:(timecolumn(1) + 24 * 3660):(0):($2/1000) w boxxyerrorbars lc "red" lw 0.1 not, \

  #FN_IN u 1:($2/1000):(24 * 3660):(0) w vectors nohead lc "red" not, \
  #FN_IN u 1:(prev0(0)):(0):($2/1000 - prev0($2/1000), prev1($2/1000)) w vectors nohead lc "red" not, \

  # Fill
  #FN_IN u 1:(0):1:(timecolumn(1) + 24 * 3660):(0):($2/1000) w boxxyerrorbars fill solid 0.5 noborder not, \

  #FN_IN u 1:0:1:($1 + 24*3600):0:($2/1000) w boxxyerrorbars not

  # boxxyerrorbars: x y xlow xhigh ylow yhigh
}
