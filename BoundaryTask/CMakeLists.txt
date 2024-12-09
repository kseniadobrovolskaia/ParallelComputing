cmake_minimum_required(VERSION 3.14)

project(BoundaryTask)

set(CMAKE_C_COMPILER /usr/bin/mpic++)
add_compile_options(-ggdb -Wall)

set(SOURCE_EXE BoundaryTask.c)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(BoundTask ${SOURCE_EXE})

target_link_libraries(BoundTask ${MPI_C_LIBRARIES})
