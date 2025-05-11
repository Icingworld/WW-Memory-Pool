# memory_benchmark.cpp
add_executable(memory_benchmark
    src/memory_benchmark.cpp
)

target_include_directories(memory_benchmark PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(memory_benchmark PRIVATE
    WW::memory
)

# analyze_benchmark.cpp
add_executable(analyze_benchmark
    src/analyze_benchmark.cpp
)

target_include_directories(analyze_benchmark PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(analyze_benchmark PRIVATE
    WW::memory
)
