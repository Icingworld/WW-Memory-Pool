cmake_minimum_required(VERSION 3.10)
project(ww-memory-pool VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(memory-pool)

option(WWTEST "Enable Test" ON)
option(WWBENCHMARK "Enable Benchmark" OFF)

if (WWTEST)
    message(STATUS "Test ON")
    add_subdirectory(third-party)
    add_subdirectory(test)
else()
    message(STATUS "Test OFF")
endif()

if (WWBENCHMARK)
    message(STATUS "Benchmark ON")
    add_subdirectory(benchmark)
else()
    message(STATUS "Benchmark OFF")
endif()
