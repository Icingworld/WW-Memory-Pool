#include <string>

#include <ThreadCache.h>
#include <gtest/gtest.h>

class ThreadCacheTest : public testing::Test
{
public:
    WW::ThreadCache & thread_cache = WW::ThreadCache::getThreadCache();
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

    // 申请一个12字节的内存
    // 实际上会对齐为16字节
    ptr = thread_cache.allocate(12);    // 换64字节可以运行
    EXPECT_NE(ptr, nullptr);
    // 尝试构造一个大对象
    LargeObject * obj = new(ptr) LargeObject;
    EXPECT_EQ(obj->d, std::string("hello, world! hello, world!"));
    obj->~LargeObject();

    // 这里会出现UB，因为内存实际上不够构造这个对象，破坏了内存，如果申请足够的内存如64字节，则能顺利运行测试
    EXPECT_DEATH({
    	// 释放内存
    	thread_cache.deallocate(ptr, 12);
    }, ".*");
}