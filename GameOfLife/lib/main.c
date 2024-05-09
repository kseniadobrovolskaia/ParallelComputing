#include <ctype.h>
#include <getopt.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "GameOfLife.h"

#define SEED 1001
// clang-format off
static struct option CmdLineOpts[] = {
    {"help",               no_argument,        0,  'h'                    }, 
    {"count-steps",        required_argument,  0,  's'                    },
    {"mesh-colunms",       required_argument,  0,  'c'                    },
    {"mesh-rows",          required_argument,  0,  'r'                    },
    {"start-mesh-file",    required_argument,  0,  'f'                    },
    {"out-mesh-file",      required_argument,  0,  'o'                    },
    {"seed",               required_argument,  0,  SEED                   },
    {0,                    0,                  0,   0                     }};
// clang-format on

int Rank;
int Cols = 0;
int Rows = 0;
long long Seed = -1;
int CountSteps = 0;
char *FileName = NULL;
char *OutputFileName = "Out.txt";

static void printHelp(const char *ProgName, int ErrorCode) {
  if (Rank == 0) {
    fprintf(stderr,
            "USAGE:     %s   [options]   -f  <start_mesh_state_file>\n\n",
            ProgName);
    fprintf(stderr, "OPTIONS: \n");
    struct option *opt = CmdLineOpts;
    while (opt->name) {
      if (isprint(opt->val))
        fprintf(stderr, "\t   -%c\t --%s\n", (char)(opt->val), opt->name);
      else
        fprintf(stderr, "\t     \t --%s\n", opt->name);
      opt += 1;
    }
  }
  MPI_Finalize();
  exit(ErrorCode);
}

/**
 * @brief setValue - it helps process incorrect values, which are given
 *                   for int argument.
 */
static void setValue(const char *NameValue, const char *GivenValue,
                     int *SetVar) {
  // find sign
  while (isspace((unsigned char)*GivenValue))
    GivenValue++;
  char Sign = *GivenValue;
  char *Endptr; //  Store the location where conversion stopped
  unsigned long TryValue = strtoul(GivenValue, &Endptr, /* base */ 10);

  if (GivenValue == Endptr) {
    if (Rank == 0)
      fprintf(stderr, "Invalid %s %s provided\n", NameValue, GivenValue);
    MPI_Abort(MPI_COMM_WORLD, 1);
  } else if (*Endptr) {
    if (Rank == 0)
      fprintf(stderr, "Extra text after %s\n", NameValue);
    MPI_Abort(MPI_COMM_WORLD, 1);
  } else if (Sign == '-' && TryValue != 0) {
    if (Rank == 0)
      fprintf(stderr, "Negative %s\n", NameValue);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  *SetVar = TryValue;
}

/**
 * @brief parseCmdLine - it parses the command line arguments.
 */
static void parseCmdLine(int Argc, char **Argv) {
  int NextOpt;
  while (1) {
    NextOpt = getopt_long(Argc, Argv,
                          "h"
                          "s:"
                          "c:"
                          "r:"
                          "o:"
                          "f:",
                          CmdLineOpts, NULL);
    if (NextOpt == -1)
      break;
    switch (NextOpt) {
    case SEED:
      setValue("seed", optarg, &Seed);
      break;
    case 's':
      setValue("count-steps", optarg, &CountSteps);
      break;
    case 'c':
      setValue("mesh-colunms", optarg, &Cols);
      break;
    case 'r':
      setValue("mesh-rows", optarg, &Rows);
      break;
    case 'h':
      printHelp(Argv[0], 0);
      break;
    case 'f': {
      FileName = (char *)calloc(strlen(optarg) + 1, sizeof(char));
      memcpy(FileName, optarg, strlen(optarg));
      break;
    }
    case 'o': {
      OutputFileName = (char *)calloc(strlen(optarg) + 1, sizeof(char));
      memcpy(OutputFileName, optarg, strlen(optarg));
      break;
    }
    default:
      printHelp(Argv[0], 1);
      break;
    }
  }
}

static void checkDefined(int Var, const char *VarName) {
  if (Var == 0) {
    if (Rank == 0)
      fprintf(stderr, "How many %s?\n", VarName);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
}

char **OwnMesh;
int OwnRows;
int OwnCols;

int main(int Argc, char **Argv) {
  int CommSize;
  double StartTime, FinishTime, Time, AllTime;

  MPI_Init(&Argc, &Argv);
  MPI_Comm_size(MPI_COMM_WORLD, &CommSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &Rank);
  MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

  parseCmdLine(Argc, Argv);

  checkDefined(Rows, "rows (-r)");
  checkDefined(Cols, "cols (-c)");
  checkDefined(CountSteps, "steps (-s)");

  //----------------------------Creating_cart_topology-----------------------------------

  int Dims[2] = {0, 0};
  MPI_Dims_create(CommSize, 2, Dims);
  int Periods[2] = {1, 1};
  int Coordinates[2];
  MPI_Comm MeshComm;
  MPI_Cart_create(MPI_COMM_WORLD, 2, Dims, Periods, 0, &MeshComm);
  MPI_Cart_coords(MeshComm, Rank, 2, Coordinates);

  //-------------------------Getting_boarder_information---------------------------------

  if (Rank == 0) {
    getProcBorders(Dims, Rows, Cols);
    if (Dims[0] > Rows || Dims[1] > Cols) {
      printf("Incorrect sizes! Dims[0]: %d, Dims[1]: %d, Rows: %d, Cols: %d\n",
             Dims[0], Dims[1], Rows, Cols);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  int RowsCols[2];
  int StartDims[2];
  MPI_Status Status;
  MPI_Recv(&RowsCols, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &Status);
  MPI_Recv(&StartDims, 2, MPI_INT, 0, 1, MPI_COMM_WORLD, &Status);
  OwnRows = RowsCols[0];
  OwnCols = RowsCols[1];

  int Offset = StartDims[0] * Cols + StartDims[1];

  //-------------------------Getting_initial_mesh_state----------------------------------

  StartTime = MPI_Wtime();
  OwnMesh = getMemory(OwnRows + 2, OwnCols + 2);

  if (FileName == NULL) {
    if (Rank == 0)
      printf(
          "No start mesh file provided. Random generation start mesh state.\n");
    if (Seed == -1 && Rank == 0) {
      Seed = time(NULL);
      printf("Seed is not provided. Using generated randomly: %llu\n", Seed);
    }
    MPI_Bcast(&Seed, 1, MPI_UNSIGNED_LONG_LONG, /* Root */ 0, MPI_COMM_WORLD);
    srand(Seed);
    for (int i = 1; i <= OwnRows; ++i) {
      for (int j = 1; j <= OwnCols; ++j) {
        if (rand() % 2)
          OwnMesh[i][j] = '1';
        else
          OwnMesh[i][j] = '0';
      }
    }
  } else {
    if (Seed != -1) {
      if (Rank == 0)
        printf("Seed (--seed) and input file (-f) can't be both provided\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (Rank == 0) {
      char CreateInputFileCmd[LENGHT_STR] = "echo -n $(tr -d \"\n\" < ";
      strcat(CreateInputFileCmd, FileName);
      strcat(CreateInputFileCmd, ") > Tmp.txt");
      system(CreateInputFileCmd);
    }
    readFile("Tmp.txt", Offset, Cols, Rank);
  }

  //---------------------------------Game_of_life----------------------------------------

  MPI_Barrier(MPI_COMM_WORLD);
  gameOfLife(Rank, Cols, Rows, Offset * sizeof(char), CountSteps, Coordinates,
             Dims[0], Dims[1]);

  FinishTime = MPI_Wtime();

  //---------------------------------Time_results----------------------------------------

  Time = FinishTime - StartTime;
  MPI_Reduce(&Time, &AllTime, 1, MPI_DOUBLE, MPI_MAX, 0, MeshComm);

  if (Rank == 0)
    printf("All time: %.3f s\n", AllTime);

  free(OwnMesh[0]);
  free(OwnMesh);
  MPI_Finalize();
}
