#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "GameOfLife.h"

void readFile(char *FileName, int Offset, int Cols, int Rank) {
  MPI_File File;
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
  int Error =
      MPI_File_open(MPI_COMM_SELF, FileName, MPI_MODE_CREATE | MPI_MODE_RDWR,
                    MPI_INFO_NULL, &File);
  if (Error != MPI_SUCCESS) {
    char ErrorString[BUFSIZ];
    int LengthString;
    MPI_Error_string(Error, ErrorString, &LengthString);
    fprintf(stderr, "%s\nError in reading\n\n", ErrorString);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  int ReadOffset;
  for (int RowIdx = 0; RowIdx < OwnRows; ++RowIdx) {
    ReadOffset = Offset + RowIdx * Cols * sizeof(char);
    MPI_File_set_view(File, ReadOffset, MPI_CHAR, MPI_CHAR, "native",
                      MPI_INFO_NULL);
    MPI_File_read_all(File, &OwnMesh[RowIdx + 1][1], OwnCols * sizeof(char),
                      MPI_CHAR, MPI_STATUS_IGNORE);
  }
  MPI_File_close(&File);
}

void writeFile(MPI_File *File, int Offset, int Cols, int Rows, int Step,
               int Rank) {
  int WriteOffset;
  for (int RowIdx = 0; RowIdx < OwnRows; ++RowIdx) {
    WriteOffset = Offset + RowIdx * Cols * sizeof(char) +
                  Step * Rows * Cols * sizeof(char);
    MPI_File_set_view(*File, WriteOffset, MPI_CHAR, MPI_CHAR, "native",
                      MPI_INFO_NULL);
    MPI_File_write_all(*File, &OwnMesh[RowIdx + 1][1], OwnCols * sizeof(char),
                       MPI_CHAR, MPI_STATUS_IGNORE);
  }
}

// clang-format off
/**
 * @brief getProcBorders - This function is called for a process with rank 0.
 *                         It is needed to distribute the all mesh between processes. 
 *                         It sends a message about the local number of rows and columns 
 *                         to be processed by one process (RowsCols). 
 *                         It also sends indexes to the first column and row from the 
 *                         global mesh (StartDims).
 * E.G.:
 *        For Rows = 5, Cols = 9, Dims = { 2, 2 }
 *
 *        Rank 0    00000 | 0000  Rank 1
 *                  00000 | 0000
 *                  00000 | 0000
                    ______|_____
 *        Rank 2    00000 | 0000  Rank 3
 *                  00000 | 0000
 *       
 *        Rank 0: RowsCols = { 3, 5 }, StartDims = { 0, 0 }
 *        Rank 1: RowsCols = { 3, 4 }, StartDims = { 0, 5 }
 *        Rank 2: RowsCols = { 2, 5 }, StartDims = { 3, 0 }
 *        Rank 3: RowsCols = { 2, 4 }, StartDims = { 3, 5 }
 */
// clang-format on
void getProcBorders(int *Dims, int Rows, int Cols) {
  static int RowsCols[2];
  static int StartDims[2];
  int PrevRows = 0;
  int PrevCols = 0;
  int LeftRows = Rows;
  int LeftCols = Cols;

  MPI_Request Request;
  int RemainderRows = Rows % Dims[0];
  int RemainderCols = Cols % Dims[1];
  for (int RowIdx = 0; RowIdx < Dims[0]; ++RowIdx) {
    if (Rows / Dims[0] > LeftRows)
      RowsCols[0] = LeftRows;
    else {
      RowsCols[0] = Rows / Dims[0];
      if (RemainderRows != 0) {
        ++RowsCols[0];
        --RemainderRows;
      }
    }
    LeftRows -= RowsCols[0];
    // StartDims - firs row and col for the process
    StartDims[0] = PrevRows;
    PrevRows += RowsCols[0];

    // Columns
    for (int ColIdx = 0; ColIdx < Dims[1]; ++ColIdx) {
      if (Cols / Dims[1] > LeftCols)
        RowsCols[1] = LeftCols;
      else {
        RowsCols[1] = Cols / Dims[1];
        if (RemainderCols != 0) {
          RowsCols[1]++;
          RemainderCols--;
        }
      }
      StartDims[1] = PrevCols;
      PrevCols += RowsCols[1];
      // Send the boundary info to process
      MPI_Isend(&RowsCols, 2, MPI_INT, RowIdx * Dims[1] + ColIdx, 0,
                MPI_COMM_WORLD, &Request);
      MPI_Isend(&StartDims, 2, MPI_INT, RowIdx * Dims[1] + ColIdx, 1,
                MPI_COMM_WORLD, &Request);
    }

    // Start processing the next process
    PrevCols = 0;
    LeftCols = Cols;
    RemainderCols = Cols % Dims[1];
  }
}

char **getMemory(int Rows, int Cols) {
  char **Result = (char **)calloc(Rows, sizeof(char *));
  for (unsigned R = 0; R < Rows; ++R)
    Result[R] = (char *)calloc(Cols, sizeof(char));
  return Result;
}
