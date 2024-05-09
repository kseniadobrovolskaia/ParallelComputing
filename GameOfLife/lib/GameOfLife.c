#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GameOfLife.h"

static void updateState(char **NewOwnMesh, int RowIdx, int ColIdx, int Result) {
  if (OwnMesh[RowIdx][ColIdx] == '1') {
    if ((Result == 2) || (Result == 3))
      NewOwnMesh[RowIdx][ColIdx] = '1';
    else
      NewOwnMesh[RowIdx][ColIdx] = '0';
  } else if (Result == 3)
    NewOwnMesh[RowIdx][ColIdx] = '1';
  else
    NewOwnMesh[RowIdx][ColIdx] = '0';
}

static void calcSummAround(int RowIdx, int ColIdx, int *Summ) {
  int Result = 0;
  for (int R = -1; R <= 1; ++R) {
    for (int C = -1; C <= 1; ++C) {
      if (!(R == 0 && C == 0))
        if (OwnMesh[RowIdx + R][ColIdx + C] == '1')
          Result++;
    }
  }
  *Summ = Result;
}

static void calcBorderCells(char **NewOwnMesh) {
  int Result;
  for (unsigned RowIdx = 1; RowIdx <= OwnRows; ++RowIdx)
    for (unsigned ColIdx = 1; ColIdx <= OwnCols; ++ColIdx) {
      if (RowIdx == 1 || RowIdx == OwnRows || ColIdx == 1 ||
          ColIdx == OwnCols) {
        calcSummAround(RowIdx, ColIdx, &Result);
        updateState(NewOwnMesh, RowIdx, ColIdx, Result);
      }
    }
}

static void calcInternalCells(char **NewOwnMesh) {
  int Result;
  for (unsigned RowIdx = 2; RowIdx <= OwnRows - 1; ++RowIdx) {
    for (unsigned ColIdx = 2; ColIdx <= OwnCols - 1; ++ColIdx) {
      calcSummAround(RowIdx, ColIdx, &Result);
      updateState(NewOwnMesh, RowIdx, ColIdx, Result);
    }
  }
}

static void printOwnMesh() {
  for (unsigned RowIdx = 0; RowIdx <= OwnRows + 1; ++RowIdx) {
    for (unsigned ColIdx = 0; ColIdx <= OwnCols + 1; ++ColIdx)
      printf("%c", OwnMesh[RowIdx][ColIdx]);
    printf("\n");
  }
}

/**
 * @brief gameOfLife - Completes `CountSteps` iterations of the game of life.
 *                     For each processor, calculates its local mesh for the
 *                     next step. To calculate the boundary cells, information
 *                     is exchanged with the boundary processors.
 */
void gameOfLife(int Rank, int Cols, int Rows, int Offset, int CountSteps,
                int *Coordinates, int ProcRows, int ProcCols) {
  char **TmpMesh;
  char **NewOwnMesh = getMemory(OwnRows + 2, OwnCols + 2);
  // Creating arrays for boundary exchanging
  char *ArrayForSendLeft = (char *)calloc(OwnRows, sizeof(char));
  char *ArrayForSendRight = (char *)calloc(OwnRows, sizeof(char));
  char *ArrayForRecvLeft = (char *)calloc(OwnRows, sizeof(char));
  char *ArrayForRecvRight = (char *)calloc(OwnRows, sizeof(char));

  // Creating file to print state on each step
  MPI_File File;
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
  char RmCmd[LENGHT_STR] = "rm -f ";
  strcat(RmCmd, OutputFileName);
  system(RmCmd);
  int Error =
      MPI_File_open(MPI_COMM_SELF, OutputFileName,
                    MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &File);
  if (Error != MPI_SUCCESS) {
    if (Rank == 0) {
      char ErrorString[BUFSIZ];
      int LengthString;
      MPI_Error_string(Error, ErrorString, &LengthString);
      fprintf(stderr, "%s\nError in writing\n\n", ErrorString);
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Main game cycle
  for (unsigned Step = 0; Step < CountSteps; ++Step) {
    for (int RowIdx = 0; RowIdx < OwnRows; ++RowIdx) {
      ArrayForSendLeft[RowIdx] = OwnMesh[RowIdx + 1][1];
      ArrayForSendRight[RowIdx] = OwnMesh[RowIdx + 1][OwnCols];
    }
    MPI_Status status;
    // Send the left side and receive the right side
    MPI_Sendrecv(ArrayForSendLeft, OwnRows * sizeof(char), MPI_CHAR,
                 Coordinates[0] * ProcCols +
                     ((Coordinates[1] + ProcCols - 1) % ProcCols),
                 0, ArrayForRecvRight, OwnRows * sizeof(char), MPI_CHAR,
                 Coordinates[0] * ProcCols + ((Coordinates[1] + 1) % ProcCols),
                 0, MPI_COMM_WORLD, &status);
    // Send top left side and receive the bottom right
    MPI_Sendrecv(&OwnMesh[1][1], 1 * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + ProcRows - 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + ProcCols - 1) % ProcCols),
                 0, &OwnMesh[OwnRows + 1][OwnCols + 1], 1 * sizeof(char),
                 MPI_CHAR,
                 ((Coordinates[0] + 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + 1) % ProcCols),
                 0, MPI_COMM_WORLD, &status);
    // Send the top side and receive the bottom side
    MPI_Sendrecv(&OwnMesh[1][1], OwnCols * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + ProcRows - 1) % ProcRows) * ProcCols +
                     Coordinates[1],
                 0, &OwnMesh[OwnRows + 1][1], OwnCols * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + 1) % ProcRows) * ProcCols + Coordinates[1],
                 0, MPI_COMM_WORLD, &status);
    // Send top right side and receive the bottom left
    MPI_Sendrecv(&OwnMesh[1][OwnCols], 1 * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + ProcRows - 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + 1) % ProcCols),
                 0, &OwnMesh[OwnRows + 1][0], 1 * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + ProcCols - 1) % ProcCols),
                 0, MPI_COMM_WORLD, &status);
    // Send the right side and receive the left side
    MPI_Sendrecv(ArrayForSendRight, OwnRows * sizeof(char), MPI_CHAR,
                 Coordinates[0] * ProcCols + ((Coordinates[1] + 1) % ProcCols),
                 0, ArrayForRecvLeft, OwnRows * sizeof(char), MPI_CHAR,
                 Coordinates[0] * ProcCols +
                     ((Coordinates[1] + ProcCols - 1) % ProcCols),
                 0, MPI_COMM_WORLD, &status);
    // Send bottom right side and receive the top left
    MPI_Sendrecv(&OwnMesh[OwnRows][OwnCols], 1 * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + 1) % ProcCols),
                 0, &OwnMesh[0][0], 1 * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + ProcRows - 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + ProcCols - 1) % ProcCols),
                 0, MPI_COMM_WORLD, &status);
    // Send the bottom side and receive the top side
    MPI_Sendrecv(&OwnMesh[OwnRows][1], OwnCols * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + 1) % ProcRows) * ProcCols + Coordinates[1],
                 0, &OwnMesh[0][1], OwnCols * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + ProcRows - 1) % ProcRows) * ProcCols +
                     Coordinates[1],
                 0, MPI_COMM_WORLD, &status);
    // Send bottom left side and receive the top
    MPI_Sendrecv(&OwnMesh[OwnRows][1], 1 * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + ProcCols - 1) % ProcCols),
                 0, &OwnMesh[0][OwnCols + 1], 1 * sizeof(char), MPI_CHAR,
                 ((Coordinates[0] + ProcRows - 1) % ProcRows) * ProcCols +
                     ((Coordinates[1] + 1) % ProcCols),
                 0, MPI_COMM_WORLD, &status);

    for (int RowIdx = 0; RowIdx < OwnRows; ++RowIdx) {
      OwnMesh[RowIdx + 1][OwnCols + 1] = ArrayForRecvRight[RowIdx];
      OwnMesh[RowIdx + 1][0] = ArrayForRecvLeft[RowIdx];
    }
    calcInternalCells(NewOwnMesh);
    MPI_Barrier(MPI_COMM_WORLD);
    calcBorderCells(NewOwnMesh);

    writeFile(&File, Offset, Cols, Rows, Step, Rank);

    TmpMesh = OwnMesh;
    OwnMesh = NewOwnMesh;
    NewOwnMesh = TmpMesh;
  }
  MPI_File_close(&File);
  free(NewOwnMesh[0]);
  free(NewOwnMesh);
}
