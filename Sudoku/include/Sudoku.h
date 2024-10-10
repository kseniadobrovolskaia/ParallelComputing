#ifndef SUDOKU_H
#define SUDOKU_H


void failWithError(std::string Msg) {
  throw std::logic_error(Msg);
}

static bool isInt(double Val) {
  return abs(round(Val) - Val) < std::numeric_limits<double>::min();
}


class SudokuGame {

  struct Cell {
    int Value = 0;
    bool Fixed = false;
  };
  
  unsigned NumSquares;
  std::vector<std::vector<Cell>> Board;

public:
  void set(const std::vector<int> &Array) {
    const auto &Squares = sqrt(Array.size());
    if (!isInt(sqrt(Squares)))
      failWithError("Введённый массив судоку не является");
    if (!std::all_of(Array.begin(), Array.end(), [Squares](const auto &Elem) { return Elem <= Squares; }))
      failWithError("Элементы массива не удовлетворяют судоку");

    NumSquares = Squares;

    Board.resize(NumSquares);
    for (auto Elem = 0u, Row = 0u; Row < NumSquares; ++Row) {
      Board[Row].resize(NumSquares);
      for (auto Col = 0u; Col < NumSquares; ++Col, ++Elem) {
        const auto &Value = Array[Elem];
        if (Value != 0)
          setValue(Row, Col, Value);
      }
    }
  }

  void print() const {
    for (int i = 0; i < NumSquares; ++i) {
      for (int j = 0; j < NumSquares; ++j) {
        std::cout << Board[i][j].Value << " ";
        if ((j + 1) % static_cast<int>(sqrt(NumSquares)) == 0 && j < NumSquares - 1) 
          std::cout << "| ";
      }
      std::cout << "\n";
      if ((i + 1) % static_cast<int>(sqrt(NumSquares)) == 0 && i < NumSquares - 1)
        std::cout << "------+-------+-------\n";
    }
  }


  void setValue(int Row, int Col, int Value) {
    Board[Row][Col].Value = Value;
    Board[Row][Col].Fixed = true;
  }

  bool isValid(int Row, int Col, int Value) const {
    // Check nums for row
#pragma omp parallel for
    for (int C = 0; C < NumSquares; ++C) {
      if (Board[Row][C].Value == Value && Board[Row][C].Fixed)
        return false;
    }

    // Check nums for column
#pragma omp parallel for
    for (int i = 0; i < NumSquares; ++i) {
      if (Board[i][Col].Value == Value && Board[i][Col].Fixed)
        return false;
    }

    // Check nums for little square
    int StartRow = Row - Row % static_cast<int>(sqrt(NumSquares));
    int StartCol = Col - Col % static_cast<int>(sqrt(NumSquares));
#pragma omp parallel for collapse(2)
    for (int i = 0; i < static_cast<int>(sqrt(NumSquares)); ++i) {
      for (int j = 0; j < static_cast<int>(sqrt(NumSquares)); ++j) {
        if (Board[i + StartRow][j + StartCol].Value == 
            Value && Board[i + StartRow][j + StartCol].Fixed)
          return false;
      }
    }

    return true;
  }

  bool solve(int row, int col) {
  if (row == NumSquares - 1 && col == NumSquares)
    return true;

  if (col == NumSquares) {
    row++;
    col = 0;
  }

  if (Board[row][col].Fixed)
    return solve(row, col + 1);
 
  for (int num = 1; num <= NumSquares; ++num) {
    if (isValid(row, col, num)) {
      Board[row][col].Value = num;
      Board[row][col].Fixed = true;
      if (solve(row, col + 1))
        return true;
      Board[row][col].Value = 0;
      Board[row][col].Fixed = false;
    }
  }
  return false;
  } 
};


#endif // SUDOKU_H
