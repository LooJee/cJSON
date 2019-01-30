cmake_minimum_required (VERSION 2.8)

project (cJSON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./output/)

aux_source_directory(./src DIR_SRCS)

add_compile_options(-g -Wall -std=c99)

add_executable(cJSON ${DIR_SRCS})
