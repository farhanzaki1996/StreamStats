// Gnuplots data generated by VarySize, VaryMemory, or VaryBins

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    cout << "usage: MakePlot [S|M|B] input-file output-file [0|1]\n";
    return 0;
  }
  if (argv[1][0] != 'S' && argv[1][0] != 'M' && argv[1][0] != 'B')
  {
    cout << "Must be S, M, or B\n";
    return 0;
  }
  FILE* file;
  char command[256];

  file = fopen("plot.p", "w");

  fprintf(file, "set term postscript eps color\n");
  fprintf(file, "set output \"%s.eps\"\n", argv[3]);

  if (argv[1][0] == 'S')
    fprintf(file, "set xlabel 'Stream Size'\n");
  else if (argv[1][0] == 'M')
    fprintf(file, "set xlabel 'Percent Memory'\n");
  else if (argv[1][0] == 'B')
    fprintf(file, "set xlabel 'Number of Bins'\n");
  fprintf(file, "set ylabel 'P-value Error'\n");

  fprintf(file, "set key box\n");
  fprintf(file, "set size .5,.5\n");
 
  if (argv[1][0] == 'S')
    fprintf(file, "set logscale x\n");
  fprintf(file, "set yrange [0:1]\n");

  if (argc <= 4)
    fprintf(file, "plot \"%s\" u 1:2 t 'GK' w linespoints lw 2\n", argv[2]);
  else
    fprintf(file, "plot \"%s\" u 1:2 t 'GK' w linespoints lw 2, \"%s\" u 1:3 t 'Q-Digest' w linespoints lw 2, \"%s\" u 1:4 t 'Reservoir-Sampling' w linespoints lw 2\n", argv[2], argv[2], argv[2]);

  fclose(file);
  
  system("gnuplot plot.p");
  sprintf(command, "convert %s.eps %s.pdf", argv[3], argv[3]);
  system(command);

  system("rm plot.p");
  return 0;
}
