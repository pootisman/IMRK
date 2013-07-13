set encoding koi8r
set terminal postscript eps enhanced "Arial,10"
set output "plot_sconst.eps"
set ytics
set xtics
set ylabel "Время работы программы, сек."
set xlabel "Кол-во. приёмников."
plot "plot_sconst" u 1:2 with lines title "С потоками.",\
     "plot_sconst" u 1:3 with lines title "Без потоков."

set output "plot_sdyn.eps"
set xlabel "Кол-во. приёмников и передатчиков."
plot "plot_sdyn" u 1:2 with lines title "С потоками.",\
     "plot_sdyn" u 1:3 with lines title "Без потоков."
