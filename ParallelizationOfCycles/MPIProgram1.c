#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ISIZE 5000
#define JSIZE 5000

void writeFile(MPI_File *File, int Rank, int StartRow, int Rows);

int main(int Argc, char **Argv) {
  char *OutputFileName = "MPIProg1.txt";
  int CommSize, Rank;
  double StartTime, FinishTime, Time, AllTime;

  MPI_Init(&Argc, &Argv);
  MPI_Comm_size(MPI_COMM_WORLD, &CommSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

  int Rows = ISIZE / CommSize;
  int StartRow = Rank * Rows;
  if (Rank == CommSize - 1)
    Rows = ISIZE - StartRow;

  MPI_File File;
  int Error =
      MPI_File_open(MPI_COMM_SELF, OutputFileName,
                    MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &File);

  if (Error != MPI_SUCCESS) {
    if (Rank == 0) {
      char ErrorString[100];
      int LengthString;
      MPI_Error_string(Error, ErrorString, &LengthString);
      fprintf(stderr, "%s\nError in writing\n\n", ErrorString);
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  StartTime = MPI_Wtime();

  writeFile(&File, Rank, StartRow, Rows);

  FinishTime = MPI_Wtime();

  Time = FinishTime - StartTime;
  MPI_Barrier(MPI_COMM_WORLD);

  if (Rank == 0)
    printf("All time: %.3f s\n", Time);

  MPI_File_set_view(File, 0, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
  MPI_File_close(&File);
  MPI_Finalize();
}

void writeFile(MPI_File *File, int Rank, int StartRow, int Rows) {
  const int WordSize = 9;
  int WriteOffset;
  for (int RowIdx = StartRow; RowIdx < StartRow + Rows; ++RowIdx) {
    WriteOffset = RowIdx * JSIZE * sizeof(char) * WordSize;

    char FilledRow[JSIZE][WordSize];
    for (int J = 0; J < JSIZE; J++)
      sprintf(FilledRow[J], "%8.5lf ", sin(2 * (10 * RowIdx + J)));

    MPI_File_write_at_all(*File, WriteOffset, (void *)FilledRow,
                          JSIZE * WordSize, MPI_CHAR, MPI_STATUS_IGNORE);
  }
}
