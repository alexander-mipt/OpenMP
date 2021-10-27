cmake_minimum_required(VERSION 3.0)
project(pqsort VERSION 1.0 LANGUAGES CXX)

set(SRC init.cpp parallel_sort.cpp qsort.cpp)

add_executable(pqsort ${SRC})
target_compile_options(pqsort PRIVATE -fopenmp -Wall -Wextra)