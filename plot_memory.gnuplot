#!/usr/local/bin/gnuplot --persist

set terminal svg size 600,250
set output ARG2
set xrange [0:]
set yrange [0:]
set xlabel "Time (s)"
set ylabel "Memory (MB)"
plot ARG1 u 0:1 w l notitle
