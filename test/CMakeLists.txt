# pagecache_test.cpp
add_executable(pagecache_test
    src/pagecache_test.cpp
)

target_link_libraries(pagecache_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)

# centralcache_test.cpp
add_executable(centralcache_test
    src/centralcache_test.cpp
)

target_link_libraries(centralcache_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)

# threadcache_test.cpp
add_executable(threadcache_test
    src/threadcache_test.cpp
)

target_link_libraries(threadcache_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)

# memory_test.cpp
add_executable(memory_test
    src/memory_test.cpp
)

target_include_directories(memory_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(memory_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)