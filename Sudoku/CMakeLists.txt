cmake_minimum_required(VERSION 3.14)

project(Sudoku)

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
add_compile_options(-std=c++20 -ggdb)

set(SOURCE_EXE lib/main.cpp
  #lib/HelpFuncs.cpp
  #            lib/Sudoku.cpp
   )

include_directories(${CMAKE_SOURCE_DIR}/include
                   )

add_executable(sudokuSolver ${SOURCE_EXE})

