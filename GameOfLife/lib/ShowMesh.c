#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// clang-format off
static struct option CmdLineOpts[] = {
    {"help",               no_argument,        0,  'h'}, 
    {"count-steps",        required_argument,  0,  's'},
    {"mesh-colunms",       required_argument,  0,  'c'},
    {"mesh-rows",          required_argument,  0,  'r'},
    {0,                    0,                  0,   0 }};
// clang-format on

int Cols = 0;
int Rows = 0;
int CountSteps = 0;

static void printHelp(const char *ProgName, int ErrorCode) {
  fprintf(stderr, "USAGE:     %s   [options]   <Mesh_file>\n\n", ProgName);
  fprintf(stderr, "OPTIONS: \n");
  struct option *opt = CmdLineOpts;
  while (opt->name) {
    if (isprint(opt->val))
      fprintf(stderr, "\t   -%c\t --%s\n", (char)(opt->val), opt->name);
    else
      fprintf(stderr, "\t     \t --%s\n", opt->name);
    opt += 1;
  }
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
    fprintf(stderr, "Invalid %s %s provided\n", NameValue, GivenValue);
    exit(EXIT_FAILURE);
  } else if (*Endptr) {
    fprintf(stderr, "Extra text after %s\n", NameValue);
    exit(EXIT_FAILURE);
  } else if (Sign == '-' && TryValue != 0) {
    fprintf(stderr, "Negative %s\n", NameValue);
    exit(EXIT_FAILURE);
  }
  *SetVar = TryValue;
}

/**
 * @brief parseCmdLine - it parses the command line arguments and returns the
 *                       index for the file that should be shown.
 */
static int parseCmdLine(int Argc, char **Argv) {
  int NextOpt;
  while (1) {
    NextOpt = getopt_long(Argc, Argv,
                          "h"
                          "s:"
                          "c:"
                          "r:",
                          CmdLineOpts, NULL);
    if (NextOpt == -1)
      break;
    switch (NextOpt) {
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
    case '?':
      printHelp(Argv[0], 1);
      break;
    }
  }
  if (optind >= Argc)
    printf("No input file in args provided");
  return optind;
}

static void checkDefined(int Var, const char *VarName) {
  if (Var == 0) {
    fprintf(stderr, "How many %s?\n", VarName);
    exit(EXIT_FAILURE);
  }
}

int main(int Argc, char **Argv) {
  int FileNameIdx = parseCmdLine(Argc, Argv);

  checkDefined(Rows, "rows (-r)");
  checkDefined(Cols, "cols (-c)");
  checkDefined(CountSteps, "steps (-s)");

  char *FileName = Argv[FileNameIdx];
  char *Buffer = (char *)calloc(Rows * Cols, sizeof(char));
  FILE *File = fopen(FileName, "r");
  if (File == NULL) {
    perror("Error opening file: ");
    exit(EXIT_FAILURE);
  }

  for (int Step = 0; Step < CountSteps; ++Step) {
    printf("\n\nStep %d\n\n", Step);
    fseek(File, SEEK_SET, CHAR_BIT * Step * Rows * Cols);
    fread(Buffer, sizeof(char), Rows * Cols, File);
    for (int i = 0; i < Rows * Cols; ++i) {
      printf("%c", Buffer[i]);
      if ((i + 1) % Cols == 0)
        printf("\n");
    }
  }
  fclose(File);
  free(Buffer);
  return 0;
}
