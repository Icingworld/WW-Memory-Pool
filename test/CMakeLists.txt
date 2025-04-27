add_executable(pagecache_test pagecache_test.cpp)

target_link_libraries(pagecache_test PRIVATE
    WW::memory
    GTest::gtest
    GTest::gtest_main
)