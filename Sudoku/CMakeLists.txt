cmake_minimum_required(VERSION 3.14)

project(Sudoku)

set(CMAKE_CXX_COMPILER /usr/bin/g++)

find_package(OpenMP REQUIRED)
if(NOT OpenMP_CXX_FOUND)
  message(SEND_ERROR "Failed to find OpenMP")
  return()
endif()



set(SOURCE_EXE lib/main.cpp
               lib/Sudoku.cpp
   )

include_directories(${CMAKE_SOURCE_DIR}/include
                   )

add_compile_options(-O3 -std=c++20 -ggdb)
add_executable(sudokuSolver ${SOURCE_EXE})
target_link_libraries(sudokuSolver OpenMP::OpenMP_CXX)

