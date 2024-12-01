#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ISIZE 5000
#define JSIZE 5000

int main(int argc, char **argv) {
  int i, j;
  FILE *ff;

  ff = fopen("SeqMPI.txt", "w");
  double *a = (double *)malloc(ISIZE * JSIZE * sizeof(double));
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i * JSIZE + j] = 10 * i + j;
    }
  }
  for (i = 2; i < ISIZE; i++) {
    for (j = 0; j < JSIZE - 3; j++) {
      a[i * JSIZE + j] = sin(5 * a[(i - 2) * JSIZE + j + 3]);
    }
  }
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      fprintf(ff, "%12.5lf ", a[i * JSIZE + j]);
    }
  }

  fclose(ff);
}
