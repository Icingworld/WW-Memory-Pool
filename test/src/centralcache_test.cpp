#include <thread>

#include <gtest/gtest.h>
#include <CentralCache.h>

class CentralCacheTest : public testing::Test
{
public:
    WW::CentralCache & central_cache = WW::CentralCache::getCentralCache();
};

TEST_F(CentralCacheTest, SingleThreadFetchAndReturn)
{
    // 申请8字节空闲内存块512个
    // 此时刚好用一页来切分，获得512个内存块
    WW::FreeObject * free_object_512 = central_cache.fetchRange(8, 512);
    WW::FreeObject * head = free_object_512;
    std::size_t count = 0;
    while (head != nullptr) {
        head = head->next();
        ++count;
    }
    EXPECT_EQ(count, 512);

    // 申请16字节空闲内存块500个
    // 需要使用两页来切分，获得500个内存块
    WW::FreeObject * free_object_500 = central_cache.fetchRange(16, 500);
    head = free_object_500;
    count = 0;
    while (head != nullptr) {
        head = head->next();
        ++count;
    }
    EXPECT_EQ(count, 500);

    // 申请16字节空闲内存块20个
    // 此时会从没使用完的页端中继续获取，但还剩余512-500=12个，最多只能获得这么多
    WW::FreeObject * free_object_20 = central_cache.fetchRange(16, 20);
    head = free_object_20;
    count = 0;
    while (head != nullptr) {
        head = head->next();
        ++count;
    }
    EXPECT_EQ(count, 12);

    // 归还512个8字节内存块
    // 此时整个页段完整，应当被直接还给页缓存
    central_cache.returnRange(8, free_object_512);

    // 归还500个16字节内存块
    // 此时不完整，还处于中心缓存中
    central_cache.returnRange(16, free_object_500);

    // 归还12个16字节内存块
    // 此时页段完整，应当被直接还给页缓存
    central_cache.returnRange(16, free_object_20);
}

TEST_F(CentralCacheTest, MultiThreadFetchAndReturn)
{
    constexpr int THREAD_NUM = 4;
    constexpr int COUNT = 1000;

    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back([i, this]() {
            std::vector<WW::FreeObject *> free_objects;

            for (int j = 0; j < COUNT; ++j) {
                // 申请8字节空闲内存块512个
                // 此时刚好用一页来切分，获得512个内存块
                WW::FreeObject * free_object = central_cache.fetchRange(8 * i + 8, WW::MAX_BLOCK_NUM);
                EXPECT_NE(free_object, nullptr);
                free_objects.emplace_back(free_object);
            }

            for (int j = 0; j < COUNT; ++j) {
                central_cache.returnRange(8 * i + 8, free_objects[j]);
            }
        });
    }

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads[i].join();
    }
}