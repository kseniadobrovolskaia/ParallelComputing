#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define ISIZE 5000
#define JSIZE 5000

int main(int Argc, char **Argv) {
  if (Argc < 2) {
    printf("Usage: %s <ThreadsCount>\n", Argv[0]);
    return 0;
  }

  double Start, End;
  FILE *ff;

  int NumThreads = atoi(Argv[1]);
  if (NumThreads <= 0) {
    printf("Thread Count should be positive\n");
    return 0;
  }

  omp_set_num_threads(NumThreads);

  ff = fopen("OpenMPProgOpenMP.txt", "w");

  const int WordSize = 9;
  int Rows = ISIZE / NumThreads;

  Start = omp_get_wtime();
#pragma omp parallel shared(ff)
  {
    int WriteOffset;
    char FilledRow[JSIZE][WordSize];
    int ThreadsNum = omp_get_num_threads();
    int Rank = omp_get_thread_num();

    int StartRow = Rank * Rows;
    if (Rank == ThreadsNum - 1)
      Rows = ISIZE - StartRow;

    for (int Row = StartRow; Row < StartRow + Rows; ++Row) {
      for (int J = 0; J < JSIZE; J++) {
        if ((Row < 5) || (J >= JSIZE - 2))
          sprintf(FilledRow[J], "%8.5lf ", (float)0);
        else
          sprintf(FilledRow[J], "%8.5lf ",
                  sin(0.005 * (10 * (Row - 5) + (J + 2))) * 1.5);
      }
      WriteOffset = Row * JSIZE * sizeof(char) * WordSize;
#pragma omp critical
      {
        fseek(ff, WriteOffset, SEEK_SET);
        fwrite((void *)FilledRow, sizeof(char) * WordSize, JSIZE, ff);
      }
    }
  }

  End = omp_get_wtime();

  fclose(ff);
  printf("Time: %f\n", End - Start);
}
