set encoding koi8r
set terminal postscript eps enhanced "Arial,10"
set output "plot_sconst.eps"
set ytics
set xtics
set ylabel "����� ������ ���������, ���."
set xlabel "���-��. ��ɣ������."
plot "plot_sconst" u 1:2 with lines title "� ��������.",\
     "plot_sconst" u 1:3 with lines title "��� �������."

set output "plot_sdyn.eps"
set xlabel "���-��. ��ɣ������ � ������������."
plot "plot_sdyn" u 1:2 with lines title "� ��������.",\
     "plot_sdyn" u 1:3 with lines title "��� �������."
