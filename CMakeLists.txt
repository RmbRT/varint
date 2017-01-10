cmake_minimum_required(VERSION 2.6)
project(VariableInteger)

set(CMAKE_C_FLAGS "-std=c99 -DRLC_DEBUG -Werror -g -fopenmp")

# Select all source files.
file(GLOB_RECURSE vi_sources ./src/*.c)

# uncomment to switch between executable and library build.
#add_library(varint ${vi_sources})
add_executable(varint ${vi_sources} main.c)

file(COPY "src/" DESTINATION "include/varint/" FILES_MATCHING PATTERN "*.h")

include_directories(./include/)