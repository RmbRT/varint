cmake_minimum_required(VERSION 2.6)
project(VariableInteger)

# Remove -DUSE_IA to remove custom inline assembly for x86.
# Remove -DNDEBUG to enable assertions.
# Remove -fopenmp to remove Open MP support.
# Remove -DCUSTOM_HEAP to remove the ([currently broken in combination with openmp] and [slower than the default heap]) custom heap.
set(CMAKE_C_FLAGS "-std=c99 -DUSA_IA -fopenmp -Wall -Wno-unused-function -Wno-unknown-pragmas -Werror -g")

# Select all source files.
file(GLOB_RECURSE vi_sources ./src/*.c)

# uncomment to switch between executable and library build.
#add_library(varint ${vi_sources})
add_executable(varint ${vi_sources} main.c)

file(COPY "src/" DESTINATION "include/varint/" FILES_MATCHING PATTERN "*.h")

include_directories(./include/)