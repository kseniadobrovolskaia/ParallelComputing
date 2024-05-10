cmake_minimum_required(VERSION 3.14)

project(TransportEquation)

set(CMAKE_C_COMPILER /usr/bin/mpic++)
add_compile_options(-ggdb -Wall)

set(SOURCE_EXE TransportEquation.c)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(TransportEq ${SOURCE_EXE})

target_link_libraries(TransportEq ${MPI_C_LIBRARIES})
