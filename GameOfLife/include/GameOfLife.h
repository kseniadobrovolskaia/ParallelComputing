#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#define LENGHT_STR 256

extern char **OwnMesh;
extern int OwnRows;
extern int OwnCols;
extern char *OutputFileName;

void gameOfLife(int Rank, int Cols, int Rows, int Offset, int CountSteps,
                int *Coordinates, int ProcRows, int ProcCols);
void getProcBorders(int *Dims, int Rows, int Cols);
char **getMemory(int Rows, int Cols);
void readFile(char *Buffer, int Offset, int Cols, int Rank);
void writeFile(MPI_File *File, int Offset, int Cols, int Rows, int Step,
               int Rank);

#endif // GAME_OF_LIFE_H
