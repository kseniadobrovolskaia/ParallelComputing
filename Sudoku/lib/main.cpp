#include <fstream>

#include "json.hpp"
#include "Sudoku.h"

namespace SudokuGame {

unsigned ThreadCount;
bool SolutionFound = false;

std::vector<int> readJsonFile(const std::string &FileName) {
  std::ifstream File(FileName);

  if (!File.is_open())
    failWithError("Ошибка при открытии файла " + std::string(FileName));

  nlohmann::json JsonData;
  File >> JsonData;

  if (!JsonData.contains("sudoku"))
    failWithError("Нет поля \"sudoku\" в json файле");

  return JsonData["sudoku"];
}

} // namespace SudokuGame

int main(int Argc, const char **Argv) {
  if (Argc < 3) {
    std::cout << "Usage: ./sudokuSolver <ThreadsCount> <SudokuFile>\n";
    exit(EXIT_SUCCESS);
  }
  try {
    auto SudokuArray = SudokuGame::readJsonFile(Argv[2]);
    SudokuGame::SudokuSolver Sudoku(SudokuArray);

    std::cout << "Начальное судоку:\n\n";
    Sudoku.print();

    std::cout << "\n\n";

    SudokuGame::ThreadCount = atoi(Argv[1]);
    if (SudokuGame::ThreadCount <= 0)
      SudokuGame::failWithError("Thread Count should be positive\n");
    omp_set_num_threads(SudokuGame::ThreadCount);

    auto Start = omp_get_wtime();

    if (Sudoku.solve()) {
      auto End = omp_get_wtime();
      std::cout << "Решение:\n\n";
      Sudoku.printSolved();
      std::cout << "\n\nTime: " << End - Start << "\n";
    } else
      std::cout << "Решение судоку невозможно!\n";
  } catch (const std::exception &Ex) {
    std::cerr << Ex.what() << "\n";
  }
  return 0;
}
