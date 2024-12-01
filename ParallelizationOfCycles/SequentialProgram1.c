#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ISIZE 5000
#define JSIZE 5000

int main(int argc, char **argv) {
  int i, j;
  FILE *ff;

  ff = fopen("Seq1.txt", "w");
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      fprintf(ff, "%8.5lf ", sin(2 * (10 * i + j)));
    }
  }
  fclose(ff);
}
