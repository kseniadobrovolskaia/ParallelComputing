cmake_minimum_required(VERSION 3.14)

project(ParallelNestedCycles)

set(CMAKE_C_COMPILER /usr/bin/gcc)


add_custom_target(Seq1 ALL
  COMMAND gcc ../SequentialProgram1.c -lm -o seq1
  COMMENT "Compile sequential program 1" VERBATIM
                  )

add_custom_target(SeqMPI ALL
  COMMAND gcc ../SequentialProgramMPI.c -lm -o seqMPI
  COMMENT "Compile sequential program MPI" VERBATIM
  )

add_custom_target(SeqOpenMP ALL
  COMMAND gcc ../SequentialProgramOpenMP.c -lm -o seqOpenMP
  COMMENT "Compile sequential program OpenMP" VERBATIM
  )


set(CMAKE_CXX_COMPILER /usr/bin/mpicc)
add_compile_options(-ggdb)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH}
                   )


add_executable(parall1MPI ../MPIProgram1.c)
add_executable(parall2MPI ../MPIProgramMPI.c)

target_link_libraries(parall1MPI ${MPI_C_LIBRARIES} m)
target_link_libraries(parall2MPI ${MPI_C_LIBRARIES} m) 


find_package(OpenMP REQUIRED)
if(NOT OpenMP_CXX_FOUND)
  message(SEND_ERROR "Failed to find OpenMP")
  return()
endif()


add_compile_options(-fopenmp)
add_executable(parall1OpenMP ../OpenMPProgram1.c)
add_executable(parall2OpenMP ../OpenMPProgramOpenMP.c)

target_link_libraries(parall1OpenMP OpenMP::OpenMP_CXX m)
target_link_libraries(parall2OpenMP OpenMP::OpenMP_CXX m)




