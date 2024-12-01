#include <assert.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ISIZE 5000
#define JSIZE 5000

void fillArray(MPI_File *File, int Workers, int Rank, int StartCol, int Cols);

int main(int Argc, char **Argv) {
  char *OutputFileName = "MPIProgMPI.txt";
  int CommSize, Rank;
  double StartTime, FinishTime, Time, AllTime;

  MPI_Init(&Argc, &Argv);
  MPI_Comm_size(MPI_COMM_WORLD, &CommSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

  assert(CommSize >= 2 && "N threads must be >= 2");
  int Rem = Rank % 2;
  int Workers = CommSize / 2 + (int)((CommSize % 2) && !Rem);
  int Cols = JSIZE / Workers;
  int StartCol = Rank / 2 * Cols;
  if (Rank == (CommSize - 1) || (Rank == (CommSize - 2)))
    Cols = JSIZE - StartCol;

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

  // Start filling array and writing results in file
  StartTime = MPI_Wtime();

  fillArray(&File, Workers, Rank, StartCol, Cols);

  MPI_Barrier(MPI_COMM_WORLD);
  FinishTime = MPI_Wtime();
  // End

  Time = FinishTime - StartTime;

  if (Rank == 0)
    printf("All time: %.3f s\n", Time);

  MPI_File_set_view(File, 0, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
  MPI_File_close(&File);
  MPI_Finalize();
}

void fillArray(MPI_File *File, int Workers, int Rank, int StartCol, int Cols) {
  int Rem = Rank % 2;
  const int WordSize = 13;
  char(*FilledRow)[WordSize];
  double ArrayForSendLeft[3];
  double ArrayForRecvRight[3];
  double *CurrRow = malloc((Cols + 3) * sizeof(double));
  double *PrevRow = malloc((Cols + 3) * sizeof(double));
  FilledRow = calloc(Cols, sizeof *FilledRow);
  for (int RowIdx = Rem; RowIdx < ISIZE; RowIdx += 2) {
    for (int J = StartCol; J < StartCol + Cols; J++) {
      if ((RowIdx < 2) || (J >= JSIZE - 3))
        CurrRow[J - StartCol] = 10 * RowIdx + J;
      else
        CurrRow[J - StartCol] = sin(5 * PrevRow[J + 3 - StartCol]);
      snprintf(FilledRow[J - StartCol], WordSize + 1, "%12.5lf ",
               CurrRow[J - StartCol]);
      FilledRow[J - StartCol][WordSize] = '\0';
    }
    if (Workers > 1) {
      // First worker don't need fill array for send
      if (Rank != 0 && Rank != 1) {
        ArrayForSendLeft[0] = CurrRow[0];
        ArrayForSendLeft[1] = CurrRow[1];
        ArrayForSendLeft[2] = CurrRow[2];
      }
      MPI_Status status;
      // First worker only recv
      if (Rank == 0 || Rank == 1)
        MPI_Recv(ArrayForRecvRight, 3, MPI_DOUBLE, Rank + 2, 0, MPI_COMM_WORLD,
                 &status);
      // Last worker only send
      else if (Rank == (Workers - 1) * 2 + Rem)
        MPI_Send(ArrayForSendLeft, 3, MPI_DOUBLE, Rank - 2, 0, MPI_COMM_WORLD);
      // Others send left three elements in current row and receive right three
      // elements
      else
        MPI_Sendrecv(ArrayForSendLeft, 3, MPI_DOUBLE, Rank - 2, 0,
                     ArrayForRecvRight, 3, MPI_DOUBLE, Rank + 2, 0,
                     MPI_COMM_WORLD, &status);
      // Last worker don't need fill three elems after last one
      if (Rank != (Workers - 1) * 2 + Rem) {
        CurrRow[Cols] = ArrayForRecvRight[0];
        CurrRow[Cols + 1] = ArrayForRecvRight[1];
        CurrRow[Cols + 2] = ArrayForRecvRight[2];
      }
    }
    PrevRow = CurrRow;
    // Write result part in file
    int WriteOffset = (RowIdx * JSIZE + StartCol) * sizeof(char) * WordSize;
    MPI_File_write_at_all(*File, WriteOffset, (void *)FilledRow,
                          Cols * WordSize, MPI_CHAR, MPI_STATUS_IGNORE);

    // Wail all workers until they fill all current Row
    MPI_Barrier(MPI_COMM_WORLD);
  }
}
