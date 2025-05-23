#include <string>
#include <thread>

#include <gtest/gtest.h>
#include <ThreadCache.h>

class ThreadCacheTest : public testing::Test
{
public:
    WW::ThreadCache & thread_cache = WW::ThreadCache::get_thread_cache();
};

struct LargeObject {
    int a;
    double b;
    char c[8];
    std::string d = "hello, world! hello, world!";
};

TEST_F(ThreadCacheTest, SingleThreadAllocateAndDeallocate)
{
    // 申请8字节的内存
    void * ptr = thread_cache.allocate(8);
    EXPECT_NE(ptr, nullptr);
    // 构造一个指针
    int * p = new(ptr) int(42);
    EXPECT_EQ(*p, 42);
    // 释放内存
    thread_cache.deallocate(ptr, 8);

    ptr = thread_cache.allocate(sizeof(LargeObject));
    EXPECT_NE(ptr, nullptr);
    LargeObject * obj = new(ptr) LargeObject;
    EXPECT_EQ(obj->d, std::string("hello, world! hello, world!"));
    obj->~LargeObject();
    thread_cache.deallocate(ptr, sizeof(LargeObject));
}

TEST_F(ThreadCacheTest, MultiThreadAllocateAndDeallocate)
{
    constexpr int THREAD_NUM = 4;
    constexpr int COUNT = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back([i, COUNT, this]() {
            std::vector<void *> ptrs;

            for (int j = 0; j < COUNT; ++j) {
                void * ptr = thread_cache.allocate(8 * i + 8);
                EXPECT_NE(ptr, nullptr);
                ptrs.emplace_back(ptr);
            }

            for (int j = 0; j < COUNT; ++j) {
                thread_cache.deallocate(ptrs[j], 8 * i + 8);
            }
        });
    }

    for (auto & thread : threads) {
        thread.join();
    }
}