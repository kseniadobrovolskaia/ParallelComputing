#include <assert.h>
#include <cstddef>
#include <ctype.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define LEN_STR_TO_PRINT_ONE_MOMENT 21
const char *SolutionsFilename = "Calculations.txt";

// Define they later in depend of level
double h = 0;
// h * M = xM - x0;
int M = 0;

#define HARD_LEVEL
#ifdef HARD_LEVEL
//_________Hard___________
const double A = 100000.0;

const double x0 = -10;
const double xM = 10;

const double a = sqrt(2);
const double b = a;

// y'' = f(y)
double f(double y) { return -A * (y * y * y - y); };

double deriv_f(double y) { return -A * (3 * y * y - 1); }
//_________________________
#else
//_________Easy____________
const double x0 = 0;
const double xM = 1;

const double a = 1;
const double b = 1;

// y'' = f(y)
double f(double y) { return exp(-y); };

double deriv_f(double y) { return -exp(-y); }
//_________________________
#endif

// y(x) = y0(x) + v(x), y0 - rough linear solution
double y0(double x) { return a + (b - a) / (xM - x0) * (x - x0); }

double f0(double x) { return f(y0(x)); }

double deriv_f0(double x) { return deriv_f(y0(x)); }

// a * v_{m-1} + b * v_m + c * v_{m+1} = g
typedef struct ReductCoeffs {
  double a;
  double b;
  double c;
  double g;
  // Use only on first reduction iteration!
  void fillInitCoeffs(double x) {
    a = 1 - deriv_f0(x - h) * h * h / 12;
    b = -2 - 5 * deriv_f0(x) * h * h / 6;
    c = 1 - deriv_f0(x + h) * h * h / 12;
    g = (f0(x + h) / 12 + 5 * f0(x) / 6 + f0(x - h) / 12) * h * h;
  };
  void fillNewCoeffs(struct ReductCoeffs Prev, struct ReductCoeffs Curr,
                     struct ReductCoeffs Next) {
    a = -Prev.a * Curr.a / Prev.b;
    b = (Curr.b * Prev.b * Next.b - Curr.a * Prev.c * Next.b -
         Prev.b * Curr.c * Next.a) /
        (Next.b * Prev.b);
    c = -Curr.c * Next.c / Next.b;
    g = (Curr.g * Prev.b * Next.b - Curr.a * Prev.g * Next.b -
         Curr.c * Prev.b * Next.g) /
        (Next.b * Prev.b);
  };
  void print() const {
    printf("\nReduction coeffs:\n    a = %.15f\n    b = %.15f\n    c = %.15f\n"
           "    g = %.15f\n------------------\n",
           a, b, c, g);
  };
} ReductCoeffs;

MPI_Datatype mpi_coeffs_type;

static int getPositivePowerOfTwo(int n) {
  if (n == 0)
    return -1;

  if (ceil(log2(n)) != floor(log2(n)))
    return -1;
  return log2(n);
}

double sample_time() {
  static double last_time = MPI_Wtime();
  double current = MPI_Wtime();
  double ret = current - last_time;
  last_time = current;
  return ret;
}

void calcBoundaryTask(int Rank, int Size);

int main(int Argc, char **Argv) {
  int Size, Rank;

#ifdef HARD_LEVEL
  // h must be << 1 / sqrt(A);
  // h * M = xM - x0, h << 1 / sqrt(A); M = 2^p + 1;
  h = 1 / sqrt(A) / 1000;
  M = (xM - x0) / h;
#else
  M = 4000;
#endif
  --M;
  for (unsigned k = 0; k <= 4; ++k)
    M |= M >> (1 << k);
  ++M;
  h = (xM - x0) / M;

  MPI_Init(&Argc, &Argv);
  MPI_Comm_size(MPI_COMM_WORLD, &Size);
  assert(Size <= 32);
  assert(getPositivePowerOfTwo(Size) >= 0);

  /* Create a type for struct ReductCoeffs */
  const int nitems = 4;
  int blocklengths[4] = {1, 1, 1, 1};
  MPI_Datatype types[4] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
  MPI_Aint offsets[4];

  offsets[0] = offsetof(ReductCoeffs, a);
  offsets[1] = offsetof(ReductCoeffs, b);
  offsets[2] = offsetof(ReductCoeffs, c);
  offsets[3] = offsetof(ReductCoeffs, g);

  MPI_Type_create_struct(nitems, blocklengths, offsets, types,
                         &mpi_coeffs_type);
  MPI_Type_commit(&mpi_coeffs_type);

  MPI_Comm_rank(MPI_COMM_WORLD, &Rank);

  sample_time();

  calcBoundaryTask(Rank, Size);

  double FullTime = sample_time();

  if (Rank == 0)
    printf("Time: %f sec.\n", FullTime);
  MPI_Type_free(&mpi_coeffs_type);
  MPI_Finalize();
  return 0;
}

void getStartAndLenX(int *StartX, int *LenX, int Rank, int Size) {
  int SegmentForOneProc = M / Size;
  int Start = SegmentForOneProc * Rank;
  int End = SegmentForOneProc * (Rank + 1);
  // We calculate M = 2^p, p > 4, this mean that M % Size == 0 if Size <= 32
  assert(M % Size == 0);
  *LenX = End - Start;
  *StartX = Start;
}

void printSolution(int StartX, int EndX, double *Solutions) {
#ifdef HARD_LEVEL
  const int Divisor = 4096 * 2 * log(A / 100);
#else
  const int Divisor = 128;
#endif
  FILE *ResultFile;
  ResultFile = fopen(SolutionsFilename, "w");
  // In result file printing y = v + y0
  for (int J = StartX; J < EndX; J += Divisor)
    fprintf(ResultFile, "%-10.5lf%-10.5lf\n", x0 + J * h,
            /* v */ Solutions[J] + y0(x0 + J * h));
  fclose(ResultFile);
}

// a Prev + b Curr + c Next = g
// This function return Curr
double solveLin(ReductCoeffs C, double Prev, double Next) {
  return (C.g - C.a * Prev - C.c * Next) / C.b;
}

void calcBoundaryTask(int Rank, int Size) {
  int StartX, LenX;
  getStartAndLenX(&StartX, &LenX, Rank, Size);
  int SegmentForOneProc = M / Size;

  double LeftBound, RightBound;
  ReductCoeffs *Coeffs =
      (ReductCoeffs *)calloc(SegmentForOneProc + 1, sizeof(ReductCoeffs));

  int p = log2(M);
  // We want 2^p = 32 >> Num procs ~ 8
  assert(p > 4);
  // Count reduction steps for calculate bounds info for each proc.
  int ReductionSteps = p - getPositivePowerOfTwo(Size);

  if (Rank == 0) {
    // Calculate and send boundary info for each proc.
    ReductCoeffs *CoeffsArray =
        (ReductCoeffs *)calloc(M + 1, sizeof(ReductCoeffs));
    // This solution for сorrection v, its boundary conditions are 0.
    double *Solution = (double *)calloc(M + 1, sizeof(double));
    Solution[0] = 0;
    Solution[M] = 0;

    for (int i = 0; i <= SegmentForOneProc; ++i)
      CoeffsArray[i].fillInitCoeffs(/* x */ i * h);
    for (int Proc = 1; Proc < Size; ++Proc) {
      // Recv initial coeffs from others
      MPI_Recv(CoeffsArray + Proc * SegmentForOneProc, SegmentForOneProc + 1,
               mpi_coeffs_type, Proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    int H = 1, PrevH = 1, RedStep = 0;
    for (; RedStep < p - 1; ++RedStep) {
      H *= 2;
      int i = 0;
      for (i = H; i < M; i += H) {
        CoeffsArray[i].fillNewCoeffs(CoeffsArray[i - PrevH], CoeffsArray[i],
                                     CoeffsArray[i + PrevH]);
      }
#ifdef HARD_LEVEL
      // Check that i_last == M, i.e. all coeffs cells filled
      assert(i == M);
#endif
      PrevH = H;
    }

    for (; RedStep >= ReductionSteps; --RedStep) {
      int i = 0;
      for (i = H; i < M; i += PrevH)
        Solution[i] =
            solveLin(CoeffsArray[i], Solution[i - H], Solution[i + H]);
#ifdef HARD_LEVEL
      // Check that i_last == M, i.e. all solutions filled
      assert(H == PrevH || M + H == i);
#endif
      PrevH = H;
      H /= 2;
    }
    LeftBound = Solution[0];
    RightBound = Solution[SegmentForOneProc];
    for (int i = 0; i <= SegmentForOneProc; ++i) {
      Coeffs[i] = CoeffsArray[i];
    }
    for (int Proc = 1; Proc < Size; ++Proc) {
      double Bounds[2] = {Solution[PrevH * Proc], Solution[PrevH * (Proc + 1)]};
      // Send boundary information for others
      MPI_Send(Bounds, 2, MPI_DOUBLE, Proc, 0, MPI_COMM_WORLD);
    }

    free(CoeffsArray);
    free(Solution);
  } else {
    for (int i = 0; i <= SegmentForOneProc; ++i)
      Coeffs[i].fillInitCoeffs(/* x */ (SegmentForOneProc * Rank + i) * h);
    MPI_Send(Coeffs, SegmentForOneProc + 1, mpi_coeffs_type, /* For 0 */ 0, 0,
             MPI_COMM_WORLD);

    int H = 1, PrevH = 1, RedStep = 0;
    for (; RedStep < ReductionSteps; ++RedStep) {
      H *= 2;
      int i = 0;
      for (i = H; i < SegmentForOneProc; i += H) {
        Coeffs[i].fillNewCoeffs(Coeffs[i - PrevH], Coeffs[i],
                                Coeffs[i + PrevH]);
      }
      PrevH = H;
    }
    // Recv from 0 proc boarder info
    double Bounds[2];
    MPI_Recv(&Bounds, 2, MPI_DOUBLE, /* From Rank */ 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    LeftBound = Bounds[0];
    RightBound = Bounds[1];
  }

  double *Solution = (double *)calloc(SegmentForOneProc + 1, sizeof(double));
  Solution[0] = LeftBound;
  Solution[SegmentForOneProc] = RightBound;

  int PrevH = SegmentForOneProc / 2, H = PrevH;
  for (; H > 0; H /= 2) {
    int i = 0;
    for (i = H; i < SegmentForOneProc; i += PrevH)
      Solution[i] = solveLin(Coeffs[i], Solution[i - H], Solution[i + H]);
    PrevH = H;
  }

  if (Rank == 0) {
    double *Result = (double *)calloc(M + 1, sizeof(double));
    for (int i = 0; i < SegmentForOneProc; ++i)
      Result[i] = Solution[i];
    // Recv results from others
    for (int Proc = 1; Proc < Size; ++Proc) {
      MPI_Recv(Result + Proc * SegmentForOneProc, SegmentForOneProc, MPI_DOUBLE,
               Proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    printSolution(0, M + 1, Result);
    free(Result);
  } else {
    // Send for 0 proc results
    MPI_Send(Solution, SegmentForOneProc, MPI_DOUBLE, /* For Rank */ 0, 0,
             MPI_COMM_WORLD);
  }
  free(Solution);
  free(Coeffs);
}
