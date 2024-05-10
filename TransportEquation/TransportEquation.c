#include <ctype.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

const double a = 1.0;

const int K = 100;
const int M = 100;

const double T = 1;
const double X = 1;

const double Tau = T / K;
const double h = X / M;

#define LEN_STR_TO_PRINT_ONE_MOMENT 31

// Initial and boundary fuctions.
double phi(double X) {
  if (X > 0.31)
    return 0;
  return sin(X * 10);
};
double psi(double Time) { return 0; };
double f(double X, double Time) { return 0; };

void calcTransportEq(int Rank, int Size);

int main(int Argc, char **Argv) {
  int Size, Rank;
  MPI_Init(&Argc, &Argv);
  MPI_Comm_size(MPI_COMM_WORLD, &Size);
  MPI_Comm_rank(MPI_COMM_WORLD, &Rank);

  double StartTime = MPI_Wtime();

  calcTransportEq(Rank, Size);

  double StopTime = MPI_Wtime();

  if (Rank == 0)
    printf("Time: %f sec.\n", StopTime - StartTime);
  MPI_Finalize();
  return 0;
}

void getStartAndLenX(int *StartX, int *LenX, int Rank, int Size) {
  int SegmentForOneProc = M / Size;
  int Start = SegmentForOneProc * Rank;
  int End = SegmentForOneProc * (Rank + 1);
  // Equally distribute the remainder between the first processors.
  if (M % Size > 0) {
    if (Rank < M % Size) {
      Start += Rank;
      End += Rank + 1;
    } else {
      Start += M % Size;
      End += M % Size;
    }
  }
  *LenX = End - Start;
  *StartX = Start;
}

void printOneMoment(MPI_File *File, int Time, int X, double U) {
  int Offset = (Time * M + X + 1) * LEN_STR_TO_PRINT_ONE_MOMENT;
  char *PrintLine = (char *)calloc(LEN_STR_TO_PRINT_ONE_MOMENT, sizeof(char));
  snprintf(PrintLine, LEN_STR_TO_PRINT_ONE_MOMENT + 2,
           "%-10.2f%-10.2f%-10.4f\n", Time * Tau, X * h, U);
  MPI_File_write_at(*File, Offset, PrintLine, LEN_STR_TO_PRINT_ONE_MOMENT,
                    MPI_CHAR, MPI_STATUS_IGNORE);
}

void calcTransportEq(int Rank, int Size) {
  MPI_File File;
  system("rm -f Calculation.txt");
  MPI_File_open(MPI_COMM_WORLD, "Calculation.txt",
                MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &File);
  MPI_File_set_size(File, 0);

  if (Rank == 0) {
    const char *init = " Time       X        U        \n";
    MPI_File_write_at(File, 0, init, LEN_STR_TO_PRINT_ONE_MOMENT, MPI_CHAR,
                      MPI_STATUS_IGNORE);
  }

  int SendSize;
  MPI_Pack_size(2 * K, MPI_DOUBLE, MPI_COMM_WORLD, &SendSize);
  double *BufferToSend = (double *)calloc(SendSize, sizeof(double));
  MPI_Buffer_attach(BufferToSend, SendSize);

  int StartX, LenX;
  getStartAndLenX(&StartX, &LenX, Rank, Size);
  double *PrevTimeLayer = (double *)calloc(LenX, sizeof(double));
  double *CurrTimeLayer = (double *)calloc(LenX, sizeof(double));

  MPI_Barrier(MPI_COMM_WORLD);

  // Init first time layer using initial conditions.
  for (int StepX = 0; StepX < LenX; ++StepX) {
    PrevTimeLayer[StepX] = phi((StartX + StepX) * h);
    printOneMoment(&File, 0, StartX + StepX, PrevTimeLayer[StepX]);
  }

  double PrevElemCurrTimeLayer = 0;
  double PrevElemPrevTimeLayer = Rank == 0 ? 0 : phi((StartX - 1) * h);
  const double FuncCoeff = 2. / (a / h + 1. / Tau);
  const double PrevCoeff = (a / h - 1. / Tau) / (a / h + 1. / Tau);

  for (int StepT = 1; StepT < K; ++StepT) {
    if (Rank == 0)
      CurrTimeLayer[0] = psi(StepT * Tau);
    else {
      MPI_Recv(&PrevElemCurrTimeLayer, 1, MPI_DOUBLE, Rank - 1, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      CurrTimeLayer[0] =
          FuncCoeff * f((StartX + 0.5) * h, (StepT - 0.5) * Tau) +
          PrevElemPrevTimeLayer +
          PrevCoeff * (PrevElemCurrTimeLayer - PrevTimeLayer[0]);
    }
    printOneMoment(&File, StepT, StartX, CurrTimeLayer[0]);
    for (int StepX = 1; StepX < LenX; ++StepX) {
      CurrTimeLayer[StepX] =
          FuncCoeff * f((StartX + StepX + 0.5) * h, (StepT - 0.5) * Tau) +
          PrevTimeLayer[StepX - 1] +
          PrevCoeff * (CurrTimeLayer[StepX - 1] - PrevTimeLayer[StepX]);
      printOneMoment(&File, StepT, StartX + StepX, CurrTimeLayer[StepX]);
    }

    double *Tmp = CurrTimeLayer;
    CurrTimeLayer = PrevTimeLayer;
    PrevTimeLayer = Tmp;

    PrevElemPrevTimeLayer = PrevElemCurrTimeLayer;
    if (Rank < Size - 1)
      MPI_Bsend(&PrevTimeLayer[LenX - 1], 1, MPI_DOUBLE, Rank + 1, 0,
                MPI_COMM_WORLD);
  }

  MPI_Buffer_detach(&BufferToSend, &SendSize);
  MPI_File_close(&File);
}
