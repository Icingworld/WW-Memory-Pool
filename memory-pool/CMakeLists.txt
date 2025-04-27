file(GLOB_RECURSE MEMORY_POOL_SOURCES "*.cpp")

add_library(memory-pool STATIC ${MEMORY_POOL_SOURCES})

target_include_directories(memory-pool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(WW::memory ALIAS memory-pool)