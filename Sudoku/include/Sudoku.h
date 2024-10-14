#ifndef SUDOKU_H
#define SUDOKU_H

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <omp.h>
#include <string>
#include <utility>
#include <vector>

#define MAX_NUM_SQUARES 25
#define STACK_SIZE 25

namespace SudokuGame {

extern unsigned ThreadCount;
extern bool SolutionFound;

void failWithError(std::string Msg);

static bool isInt(double Val) {
  return abs(round(Val) - Val) < std::numeric_limits<double>::min();
}

class SudokuSolver {

public:
  struct Cell {
    int Value = 0;
    std::bitset<MAX_NUM_SQUARES> PossibleValues;
  };

  struct Board {
    std::pair<int, int> CurrentIdx;
    std::vector<std::vector<Cell>> Grid;

    Board(const Board &Rhs);
    Board() = default;
    Board &operator=(const Board &Rhs) {
      if (this == &Rhs)
        return *this;

      CurrentIdx = Rhs.CurrentIdx;
      assert(!Rhs.Grid.empty());
      auto Size = Rhs.Grid.size();
      if (Grid.size())
        Grid.clear();
      Grid.resize(Size);
      for (auto Row = 0; Row < Size; ++Row) {
        Grid[Row].resize(Size);
        Grid[Row] = Rhs.Grid[Row];
      }

      return *this;
    }
  };

private:
  unsigned NumSquares;
  unsigned LittleSqDim;
  Board OriginalGrid;
  Board SolvedGrid;

public:
  SudokuSolver(const std::vector<int> &Array);

  const Board &getOriginalGrid() const { return OriginalGrid; }
  unsigned getNumSquares() const { return NumSquares; }
  void print() const;
  void printSolved() const;
  void print(const Board &Brd) const;
  void printPossibleValues(const Board &Brd) const;

  bool solve();

private:
  bool isCorrectInput(const Board &Brd) const;

  void setValue(Board &Brd, int Row, int Col, int Value) const {
    Brd.Grid[Row][Col].Value = Value;
  }

  void setPossibleValues(Board &Brd, int Row, int Col) const;

  bool reducePossibleValues(Board &Brd, int Row, int Col) const;

  // Humanistic alghorithm
  bool solveHumanistic(Board &Brd) const;
  bool eliminate(Board &Brd) const;
  bool setLoneRangersRow(Board &Brd) const;
  bool setLoneRangersColumn(Board &Brd) const;
  bool setLoneRangersLittleSquare(Board &Brd) const;
  bool setTwinsRow(Board &Brd) const;
  bool setTwinsColumn(Board &Brd) const;

  // Brute Force alghorithm
  bool fillPermutationStack(Board &Brd);
  bool solveBruteForce(Board &Brd);
  std::pair<int, int> getLeastUnsureCell(const Board &Brd) const;
  void pushIdxPermutations(const std::pair<int, int> &Idx, Board Brd,
                           std::vector<Board> *Stack) const;

public:
  bool isSolved(const Board &Grid) const;
};

} // namespace SudokuGame

#endif // SUDOKU_H
