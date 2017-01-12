cmake_minimum_required(VERSION 2.6)
project(VariableInteger)

# Remove -DUSE_IA to remove custom inline assembly for x86
# Remove -DNDEBUG to enable assertions
# Remove -fopenmp to remove Open MP support
set(CMAKE_C_FLAGS "-std=c99 -fopenmp -DUSE_IA -Wall -Werror -g -DNDEBUG")

# Select all source files.
file(GLOB_RECURSE vi_sources ./src/*.c)

# uncomment to switch between executable and library build.
#add_library(varint ${vi_sources})
add_executable(varint ${vi_sources} main.c)

file(COPY "src/" DESTINATION "include/varint/" FILES_MATCHING PATTERN "*.h")

include_directories(./include/)