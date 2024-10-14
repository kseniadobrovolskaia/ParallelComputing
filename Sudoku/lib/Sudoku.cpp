#include "Sudoku.h"

namespace SudokuGame {

std::vector<SudokuSolver::Board> PermutationsStack;

void failWithError(std::string Msg) { throw std::logic_error(Msg); }

SudokuSolver::Board::Board(const Board &Rhs) {
  auto NumSquares = Rhs.Grid.size();
  Grid.resize(NumSquares);
  for (auto Row = 0u; Row < NumSquares; ++Row) {
    Grid[Row].resize(NumSquares);
    for (auto Col = 0u; Col < NumSquares; ++Col) {
      Grid[Row][Col].Value = Rhs.Grid[Row][Col].Value;
      Grid[Row][Col].PossibleValues = Rhs.Grid[Row][Col].PossibleValues;
    }
  }
}

SudokuSolver::SudokuSolver(const std::vector<int> &Array) {
  const auto &Squares = sqrt(Array.size());
  if (!isInt(sqrt(Squares)))
    failWithError("Введённый массив судоку не является");
  if (!std::all_of(Array.begin(), Array.end(),
                   [Squares](const auto &Elem) { return Elem <= Squares; }))
    failWithError("Элементы массива не удовлетворяют правилам судоку");

  NumSquares = Squares;
  LittleSqDim = int(sqrt(Squares));

  OriginalGrid.Grid.resize(NumSquares);
  for (auto Elem = 0u, Row = 0u; Row < NumSquares; ++Row) {
    OriginalGrid.Grid[Row].resize(NumSquares);
    for (auto Col = 0u; Col < NumSquares; ++Col, ++Elem)
      setValue(OriginalGrid, Row, Col, Array[Elem]);
  }
  if (!isCorrectInput(OriginalGrid))
    failWithError("Некорректное начальное судоку");
}

bool SudokuSolver::isCorrectInput(const Board &Brd) const {
  std::vector<bool> ValuesBeen(NumSquares, false);

  auto clearValuesBeen = [&]() {
    ValuesBeen.clear();
    ValuesBeen.resize(NumSquares, false);
  };

  auto checkRowCol = [&](auto Row, auto Col) {
    auto Value = Brd.Grid[Row][Col].Value;
    if (Value == 0)
      return true;
    if (ValuesBeen[Value - 1] == true)
      return false;
    ValuesBeen[Value - 1] = true;
    return true;
  };

  // Check rows
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0; Col < NumSquares; ++Col)
      if (!checkRowCol(Row, Col))
        return false;
    clearValuesBeen();
  }

  // Check columns
  for (auto Col = 0; Col < NumSquares; ++Col) {
    for (auto Row = 0; Row < NumSquares; ++Row)
      if (!checkRowCol(Row, Col))
        return false;
    clearValuesBeen();
  }

  if (!ValuesBeen.empty())
    ValuesBeen.clear();
  ValuesBeen.resize(NumSquares, false);
  // Check little squares
  for (auto SqRow = 0; SqRow < NumSquares; SqRow = SqRow + LittleSqDim) {
    for (auto SqCol = 0; SqCol < NumSquares; SqCol = SqCol + LittleSqDim) {
      for (auto Row = SqRow; Row < SqRow + LittleSqDim; ++Row) {
        for (auto Col = SqCol; Col < SqCol + LittleSqDim; ++Col)
          if (!checkRowCol(Row, Col))
            return false;
      }
      clearValuesBeen();
    }
  }
  return true;
}

bool SudokuSolver::isSolved(const Board &Grid) const {
  std::vector<bool> ValuesBeen(NumSquares, false);

  auto clearValuesBeen = [&]() {
    ValuesBeen.clear();
    ValuesBeen.resize(NumSquares, false);
  };

  auto checkLess = [&](auto Row, auto Col) {
    auto Value = Grid.Grid[Row][Col].Value;
    if (ValuesBeen[Value - 1] == true)
      failWithError("Repeat value " + std::to_string(Value) + " on " +
                    std::to_string(Row) + " row, col or little square");
    ValuesBeen[Value - 1] = true;
    return true;
  };
  auto checkRowCol = [&](auto Row, auto Col) {
    auto Value = Grid.Grid[Row][Col].Value;
    if (Value == 0)
      return /* Cell unfilled */ false;
    if (OriginalGrid.Grid[Row][Col].Value &&
        (Value != OriginalGrid.Grid[Row][Col].Value))
      failWithError("Value in (" + std::to_string(Row) + ", " +
                    std::to_string(Col) + ") cell differ from original matrix");
    return checkLess(Row, Col);
  };

  // Check rows
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0; Col < NumSquares; ++Col)
      if (!checkRowCol(Row, Col))
        return false;
    clearValuesBeen();
  }

  // Check columns
  for (auto Col = 0; Col < NumSquares; ++Col) {
    for (auto Row = 0; Row < NumSquares; ++Row)
      if (!checkLess(Row, Col))
        return false;
    clearValuesBeen();
  }

  // Check little squares
  for (auto SqRow = 0; SqRow < NumSquares; SqRow = SqRow + LittleSqDim) {
    for (auto SqCol = 0; SqCol < NumSquares; SqCol = SqCol + LittleSqDim) {
      for (auto Row = SqRow; Row < SqRow + LittleSqDim; ++Row) {
        for (auto Col = SqCol; Col < SqCol + LittleSqDim; ++Col)
          if (!checkLess(Row, Col))
            return false;
      }
      clearValuesBeen();
    }
  }
  return true;
}

void SudokuSolver::print(const Board &Brd) const {
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0; Col < NumSquares; ++Col) {
      std::cout << Brd.Grid[Row][Col].Value;
      if (Brd.Grid[Row][Col].Value / 10u == 0)
        std::cout << "  ";
      else
        std::cout << " ";
      if ((Col + 1) % LittleSqDim == 0 && Col < NumSquares - 1)
        std::cout << "| ";
    }
    std::cout << "\n";
    if ((Row + 1) % LittleSqDim == 0 && Row < NumSquares - 1) {
      for (auto Idx = 0u; Idx <= NumSquares + 1; ++Idx)
        if ((Idx + 1) % (LittleSqDim + 1) == 0)
          std::cout << "+-";
        else
          std::cout << "---";
      std::cout << "\n";
    }
  }
}

void SudokuSolver::printPossibleValues(const Board &Brd) const {
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0; Col < NumSquares; ++Col) {
      std::cout << Brd.Grid[Row][Col].Value << " ";
      std::cout << "Possible values: (" << Brd.Grid[Row][Col].PossibleValues
                << ") ";
      if ((Col + 1) % LittleSqDim == 0 && Col < NumSquares - 1)
        std::cout << "| ";
    }
    std::cout << "\n";
    if ((Row + 1) % LittleSqDim == 0 && Row < NumSquares - 1)
      std::cout << "------+-------+-------\n";
  }
}

void SudokuSolver::print() const { print(OriginalGrid); }

void SudokuSolver::printSolved() const {
  assert(!SolvedGrid.Grid.empty());
  print(SolvedGrid);
}

static unsigned whichSet(const auto &Bitset, unsigned Count) {
  for (auto Bit = 0; Bit < Count; ++Bit)
    if (Bitset[Bit])
      return Bit + 1;
  failWithError("None set");
  return 0;
}

bool SudokuSolver::eliminate(Board &Brd) const {
  int IsChange = false;
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0; Col < NumSquares; ++Col) {
      auto PossibleVals = Brd.Grid[Row][Col].PossibleValues;
      if (Brd.Grid[Row][Col].Value == 0 && PossibleVals.count() == 1) {
        IsChange = true;
        Brd.Grid[Row][Col].Value = whichSet(PossibleVals, NumSquares);
        if (!reducePossibleValues(Brd, Row, Col))
          failWithError("Eleminate end bad");
      }
    }
  }
  return IsChange;
}

bool SudokuSolver::setLoneRangersRow(Board &Brd) const {
  auto IsChange = false, ResultChange = false;

  // Repeat if changed
  do {
    IsChange = false;
    for (auto Row = 0; Row < NumSquares; ++Row) {
      for (auto Val = 0u; Val < NumSquares; ++Val) {
        int CountVals = 0u;
        int RowLR, ColLR;
        // Find in current row
        for (auto Col = 0; Col < NumSquares; ++Col) {
          if (Brd.Grid[Row][Col].Value == 0 &&
              (Brd.Grid[Row][Col].PossibleValues[Val])) {
            ++CountVals;
            if (CountVals > 1)
              break;

            RowLR = Row;
            ColLR = Col;
          }
        }

        if (CountVals == 1) {
          // This value possible only in one cell in row. Set it.
          Brd.Grid[RowLR][ColLR].Value = Val + 1;
          reducePossibleValues(Brd, RowLR, ColLR);

          IsChange = true;
          ResultChange = true;
        }
      }
    }
  } while (IsChange);
  return ResultChange;
}

bool SudokuSolver::setLoneRangersColumn(Board &Brd) const {
  auto IsChange = false, ResultChange = false;

  // Repeat if changed
  do {
    IsChange = false;
    for (auto Col = 0; Col < NumSquares; ++Col) {
      for (auto Val = 0u; Val < NumSquares; ++Val) {
        int CountVals = 0u;
        int RowLR, ColLR;
        // Find in current col
        for (auto Row = 0; Row < NumSquares; ++Row) {
          if (Brd.Grid[Row][Col].Value == 0 &&
              (Brd.Grid[Row][Col].PossibleValues[Val])) {
            ++CountVals;
            if (CountVals > 1)
              break;

            RowLR = Row;
            ColLR = Col;
          }
        }

        if (CountVals == 1) {
          // This value possible only in one cell in row. Set it.
          Brd.Grid[RowLR][ColLR].Value = Val + 1;
          if (!reducePossibleValues(Brd, RowLR, ColLR))
            failWithError("Lone ranger row end bad");

          IsChange = true;
          ResultChange = true;
        }
      }
    }
  } while (IsChange);
  return ResultChange;

  return false;
}

bool SudokuSolver::setLoneRangersLittleSquare(Board &Brd) const {
  auto IsChange = false, ResultChange = false;

  // Repeat if changed
  do {
    IsChange = false;
    for (auto Val = 0u; Val < NumSquares; ++Val) {
      for (auto SqRow = 0; SqRow < NumSquares; SqRow += LittleSqDim) {
        for (auto SqCol = 0; SqCol < NumSquares; SqCol += LittleSqDim) {
          int CountVals = 0u;
          int RowLR, ColLR;
          // Find in current little square
          for (auto Row = SqRow; Row < SqRow + LittleSqDim; ++Row) {
            for (auto Col = 0; Col < SqCol + LittleSqDim; ++Col) {
              if (Brd.Grid[Row][Col].Value == 0 &&
                  (Brd.Grid[Row][Col].PossibleValues[Val])) {
                ++CountVals;
                if (CountVals > 1)
                  break;

                RowLR = Row;
                ColLR = Col;
              }
            }
            if (CountVals > 1)
              break;
          }

          if (CountVals == 1) {
            // This value possible only in one cell in row. Set it.
            Brd.Grid[RowLR][ColLR].Value = Val + 1;
            if (!reducePossibleValues(Brd, RowLR, ColLR))
              failWithError("Lone ranger little square end bad");

            IsChange = true;
            ResultChange = true;
          }
        }
      }
    }
  } while (IsChange);
  return ResultChange;
}

bool SudokuSolver::setTwinsRow(Board &Brd) const {
  auto IsChange = false;
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0u; Col < NumSquares; ++Col) {
      auto PossibleVals = Brd.Grid[Row][Col].PossibleValues;
      if (PossibleVals.count() == 2) {
        // Find twin
        for (auto ColTw = Col + 1; ColTw < NumSquares; ++ColTw) {
          if (PossibleVals == Brd.Grid[Row][ColTw].PossibleValues) {
            // Remove the pair of possible values from all unset cells in row
            for (auto ColRem = 0; ColRem < NumSquares; ++ColRem) {
              if (ColRem != Col && ColRem != ColTw &&
                  Brd.Grid[Row][ColRem].Value == 0 &&
                  (Brd.Grid[Row][ColRem].PossibleValues & PossibleVals).any()) {
                Brd.Grid[Row][ColRem].PossibleValues &= ~PossibleVals;
                IsChange = true;
              }
            }
          }
        }
      }
    }
  }
  return IsChange;
}

bool SudokuSolver::setTwinsColumn(Board &Brd) const {
  auto IsChange = false;
  for (auto Col = 0u; Col < NumSquares; ++Col) {
    for (auto Row = 0; Row < NumSquares; ++Row) {
      auto PossibleVals = Brd.Grid[Row][Col].PossibleValues;
      if (PossibleVals.count() == 2) {
        // Find twin
        for (auto RowTw = Row + 1; RowTw < NumSquares; ++RowTw) {
          if (PossibleVals == Brd.Grid[RowTw][Col].PossibleValues) {
            // Remove the pair of possible values from all unset cells in col
            for (auto RowRem = 0; RowRem < NumSquares; ++RowRem) {
              if (RowRem != Row && RowRem != RowTw &&
                  Brd.Grid[RowRem][Col].Value == 0 &&
                  (Brd.Grid[RowRem][Col].PossibleValues & PossibleVals).any()) {
                Brd.Grid[RowRem][Col].PossibleValues &= ~PossibleVals;
                IsChange = true;
              }
            }
          }
        }
      }
    }
  }
  return IsChange;
}

bool SudokuSolver::solveHumanistic(Board &Brd) const {
  auto IsChange = false;
  do {
    for (auto Row = 0; Row < NumSquares; ++Row) {
      for (auto Col = 0; Col < NumSquares; ++Col)
        if (!reducePossibleValues(Brd, Row, Col))
          return false;
    }
    IsChange = eliminate(Brd);
    if (!IsChange) {
      IsChange = setLoneRangersRow(Brd);
      if (!IsChange) {
        IsChange = setLoneRangersColumn(Brd);
        if (!IsChange) {
          IsChange = setLoneRangersLittleSquare(Brd);
          if (!IsChange) {
            IsChange = setTwinsRow(Brd);
            if (!IsChange)
              IsChange = setTwinsColumn(Brd);
          }
        }
      }
    }
  } while (IsChange);
  if (!isCorrectInput(Brd))
    return false;

  return true;
}

void SudokuSolver::pushIdxPermutations(const std::pair<int, int> &Idx,
                                       Board Brd,
                                       std::vector<Board> *Stack) const {
  auto [Row, Col] = Idx;
  auto PossibleVals = Brd.Grid[Row][Col].PossibleValues;
  Brd.CurrentIdx = {Row, Col};
  auto Val = 1;
  do {
    if (PossibleVals[0]) {
      Brd.Grid[Row][Col].Value = Val;
      reducePossibleValues(Brd, Row, Col);
      if (isCorrectInput(Brd))
        Stack->push_back(Brd);
    }
    PossibleVals >>= 1;
    ++Val;
  } while (PossibleVals.count());
}

bool SudokuSolver::solveBruteForce(Board &Brd) {
#pragma omp parallel shared(SolutionFound, SolvedGrid)
  {
    std::vector<Board> LocalStack;
    Board CurrentBrd;

    auto ThreadsNum = omp_get_num_threads();
    for (auto NumGrid = omp_get_thread_num();
         NumGrid < PermutationsStack.size() && !SolutionFound;
         NumGrid += ThreadsNum) {
      CurrentBrd = PermutationsStack[NumGrid];
      LocalStack.push_back(CurrentBrd);

      do {
        while (!solveHumanistic(CurrentBrd) && !LocalStack.empty()) {
          CurrentBrd = LocalStack.back();
          LocalStack.pop_back();
        }

        // Search next cell
        auto [Row, Col] = getLeastUnsureCell(CurrentBrd);
        if (Row == NumSquares) {
#pragma omp critical
          {
            SolutionFound = true;
            SolvedGrid = CurrentBrd;
          }
          break;
        }

        pushIdxPermutations(std::pair(Row, Col), CurrentBrd, &LocalStack);

        if (!LocalStack.empty()) {
          CurrentBrd = LocalStack.back();
          LocalStack.pop_back();
        }
      } while (!LocalStack.empty() && !SolutionFound);
    }
  }
  return true;
}

bool SudokuSolver::fillPermutationStack(Board &Brd) {
  Board CurrentBrd;

  std::vector<Board> Stack1Data, Stack2Data;
  std::vector<Board> *Stack1, *Stack2;

  Brd.CurrentIdx = {0, 0};
  Stack1Data.push_back(Brd);

  auto Step = 0;
  do {
    if (Step % 2) {
      Stack1 = &Stack2Data;
      Stack2 = &Stack1Data;
    } else {
      Stack1 = &Stack1Data;
      Stack2 = &Stack2Data;
    }
    while (!Stack1->empty()) {
      CurrentBrd = Stack1->back();
      Stack1->pop_back();

      // Search next cell
      auto [Row, Col] = getLeastUnsureCell(CurrentBrd);
      if (Row == NumSquares) {
        SolutionFound = true;
        SolvedGrid = CurrentBrd;
        return true;
      }
      pushIdxPermutations(std::pair(Row, Col), CurrentBrd, Stack2);
    }

    // No solutions
    if (Stack2->empty())
      return false;
    ++Step;
  } while (Stack2->size() < ThreadCount);

  PermutationsStack.resize(Stack2->size());
  PermutationsStack = *Stack2;

  return true;
}

std::pair<int, int> SudokuSolver::getLeastUnsureCell(const Board &Brd) const {
  std::pair<int, int> Idx = {0, 0};
  int Min = NumSquares + 1;
  bool IsExist = false;
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0; Col < NumSquares; ++Col) {
      if (Brd.Grid[Row][Col].Value == 0 &&
          Brd.Grid[Row][Col].PossibleValues.count() < Min) {
        IsExist = true;
        Idx = {Row, Col};
        Min = Brd.Grid[Row][Col].PossibleValues.count();
      }
    }
  }
  if (!IsExist)
    return {NumSquares, NumSquares};
  assert(Min < NumSquares + 1);
  return Idx;
}

bool SudokuSolver::solve() {
  Board StartBoard(OriginalGrid);

  // Setting all possible values
  for (auto Row = 0; Row < NumSquares; ++Row) {
    for (auto Col = 0; Col < NumSquares; ++Col)
      setPossibleValues(StartBoard, Row, Col);
  }

  StartBoard.CurrentIdx = {0, 0};

  // First part -- humanistic algorithm
  if (!solveHumanistic(StartBoard))
    return false;

  // If solved -> return true
  if (isSolved(StartBoard)) {
    SolvedGrid.Grid = StartBoard.Grid;
    return true;
  }

  if (!fillPermutationStack(StartBoard))
    return false;
  if (SolutionFound)
    return true;
  // If the humanistic algorithm returns a board with unfilled
  // cells left, then we pass it to the brute force algorithm
  if (!solveBruteForce(StartBoard))
    return false;

  // If solved -> return true
  if (SolutionFound && isSolved(SolvedGrid))
    return true;
  return false;
}

void SudokuSolver::setPossibleValues(Board &Brd, int Row, int Col) const {
  auto Value = Brd.Grid[Row][Col].Value;
  if (Value == 0) {
    for (auto Val = 1u; Val <= NumSquares; ++Val)
      Brd.Grid[Row][Col].PossibleValues.set(Val - 1);
    return;
  }
  Brd.Grid[Row][Col].PossibleValues.set(Value - 1);
}

bool SudokuSolver::reducePossibleValues(Board &Brd, int Row, int Col) const {
  assert(Brd.Grid.size() == NumSquares);
  if (Brd.Grid[Row][Col].Value > 0) {
    Brd.Grid[Row][Col].PossibleValues.reset();
    Brd.Grid[Row][Col].PossibleValues.set(Brd.Grid[Row][Col].Value - 1);
    assert(Brd.Grid[Row][Col].PossibleValues.count() == 1);
    return true;
  }
  auto SqRowSt = (Row / LittleSqDim) * LittleSqDim,
       SqColSt = (Col / LittleSqDim) * LittleSqDim;
  for (auto Idx = 0; Idx < NumSquares; Idx++) {
    auto SqRow = Idx / LittleSqDim;
    auto SqCol = Idx % LittleSqDim;
    auto Value = Brd.Grid[Row][Idx].Value;
    // Check row
    if (Value > 0)
      Brd.Grid[Row][Col].PossibleValues.reset(Value - 1);

    Value = Brd.Grid[Idx][Col].Value;
    // Check column
    if (Value > 0)
      Brd.Grid[Row][Col].PossibleValues.reset(Value - 1);

    Value = Brd.Grid[SqRowSt + SqRow][SqColSt + SqCol].Value;
    // Check little square
    if (Value > 0)
      Brd.Grid[Row][Col].PossibleValues.reset(Value - 1);
  }
  if (Brd.Grid[Row][Col].PossibleValues.none())
    return /* No possible values */ false;
  return true;
}

} // namespace SudokuGame
