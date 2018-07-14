set term png size 1600 , 1000
set output "output.png"
plot 'ag_plt' with line title 'algoritmo genetico','si_plt' with line title 'sistema inmunologico';
