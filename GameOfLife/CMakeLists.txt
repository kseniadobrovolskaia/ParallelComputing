cmake_minimum_required(VERSION 3.14)

project(GameOfLife)

set(CMAKE_CXX_COMPILER /usr/bin/mpicc)
add_compile_options(-ggdb)

set(SOURCE_EXE lib/main.c
               lib/HelpFuncs.c
               lib/GameOfLife.c
   )

find_package(MPI)
include_directories(${CMAKE_SOURCE_DIR}/include
                    SYSTEM ${MPI_INCLUDE_PATH}
                   )

add_executable(gameOfLife ${SOURCE_EXE})
add_executable(showMesh lib/ShowMesh.c)

target_link_libraries(gameOfLife ${MPI_C_LIBRARIES})
