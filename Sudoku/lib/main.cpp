#include <omp.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>

#include "json.hpp"
#include "Sudoku.h"



void readJsonFile(const std::string &FileName, SudokuGame &Sudoku) {
  std::ifstream File(FileName);

  if (!File.is_open())
    failWithError("Ошибка при открытии файла " + std::string(FileName));

  nlohmann::json JsonData;
  File >> JsonData;

  if (!JsonData.contains("sudoku"))
    failWithError("Нет поля \"sudoku\" в json файле");

  const auto &SudokuArray = JsonData["sudoku"];
  Sudoku.set(SudokuArray);
}


int main() {
  try {
    SudokuGame Sudoku;
    readJsonFile("sudoku.json", Sudoku);

    std::cout << "Начальное судоку:\n\n";
    Sudoku.print();

    std::cout << "\n\n";
    if (Sudoku.solve(0, 0)) {
      std::cout << "Решение:\n\n";
      Sudoku.print();
    } else
      std::cout << "Решение судоку невозможно!\n";
  } catch (const std::exception &Ex) {
    std::cerr << Ex.what() << "\n";
  }
  return 0;
}

