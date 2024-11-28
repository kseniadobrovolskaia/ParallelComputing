#include <stdio.h>
#include <stdlib.h>
#define ISIZE 1000
#define JSIZE 1000

double a[ISIZE][JSIZE], b[ISIZE][JSIZE];
int main(int argc, char **argv) {
  int i, j;
  FILE *ff;
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i][j] = 10 * i + j;
      b[i][j] = 0;
    }
  }
  ///начало измерения времени
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i][j] = sin(0.005 * a[i][j]);
    }
  }
  for (i = 5; i < ISIZE; i++) {
    for (j = 0; j < JSIZE - 2; j++) {
      b[i][j] = a[i - 5][j + 2] * 1.5;
    }
  }
  ///окончание измерения времени
  ff = fopen("result.txt", "w");
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      fprintf(ff, "%f ", b[i][j]);
    }
    fprintf(ff, "\n");
  }
  fclose(ff);
}
