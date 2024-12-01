#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ISIZE 5000
#define JSIZE 5000

int main(int argc, char **argv) {
  int i, j;
  FILE *ff;
  double *a = (double *)malloc(ISIZE * JSIZE * sizeof(double));
  double *b = (double *)malloc(ISIZE * JSIZE * sizeof(double));
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i * JSIZE + j] = 10 * i + j;
      b[i * JSIZE + j] = 0;
    }
  }
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i * JSIZE + j] = sin(0.005 * a[i * JSIZE + j]);
    }
  }
  for (i = 5; i < ISIZE; i++) {
    for (j = 0; j < JSIZE - 2; j++) {
      b[i * JSIZE + j] = a[(i - 5) * JSIZE + j + 2] * 1.5;
    }
  }
  ff = fopen("SeqOpenMP.txt", "w");
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      fprintf(ff, "%f ", b[i * JSIZE + j]);
    }
    fprintf(ff, "\n");
  }
  fclose(ff);
}
