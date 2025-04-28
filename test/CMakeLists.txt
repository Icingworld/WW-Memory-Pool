# pagecache_test.cpp
add_executable(pagecache_test pagecache_test.cpp)

target_link_libraries(pagecache_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)

# centralcache_test.cpp
add_executable(centralcache_test centralcache_test.cpp)

target_link_libraries(centralcache_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)

# threadcache_test.cpp
add_executable(threadcache_test threadcache_test.cpp)

target_link_libraries(threadcache_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)