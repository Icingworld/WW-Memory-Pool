#include <thread>

#include <gtest/gtest.h>
#include <PageCache.h>

class PageCacheTest : public testing::Test
{
public:
    WW::PageCache & page_cache = WW::PageCache::getPageCache();
};

TEST_F(PageCacheTest, SingleThreadFetchAndReturn)
{
    // 申请一个4页的页段
    // 此时页缓存中应当有一个124页的页段
    WW::Span * span_4 = page_cache.fetchSpan(4);
    // 模拟切分页段为内存块
    span_4->setUsed(64);
    EXPECT_NE(span_4, nullptr);
    EXPECT_EQ(span_4->count(), 4);

    // 申请一个24页的页段
    // 此时页缓存中应当有一个100页的页段
    WW::Span * span_24 = page_cache.fetchSpan(24);
    // 模拟切分页段为内存块
    span_24->setUsed(64);
    EXPECT_NE(span_24, nullptr);
    EXPECT_EQ(span_24->count(), 24);

    // 申请一个50页的页段
    // 此时页缓存中应当有一个50页的页段
    WW::Span * span_50 = page_cache.fetchSpan(50);
    // 模拟切分页段为内存块
    span_50->setUsed(64);
    EXPECT_NE(span_50, nullptr);
    EXPECT_EQ(span_50->count(), 50);

    // 归还一个24页的页段后，页缓存中应当有一个24页的页段和一个50页的页段
    span_24->setUsed(0);
    page_cache.returnSpan(span_24);

    // 归还一个4页的页段后，页缓存中应当有一个28页的页段和一个50页的页段
    span_4->setUsed(0);
    page_cache.returnSpan(span_4);

    // 归还一个50页的页段后，页缓存中应当有一个128页的页段
    span_50->setUsed(0);
    page_cache.returnSpan(span_50);
}

TEST_F(PageCacheTest, MultiThreadFetchAndReturn)
{
    constexpr int THREAD_NUM = 4;
    constexpr int COUNT = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back([i]() {
            // 获取页缓存实例
            WW::PageCache & page_cache = WW::PageCache::getPageCache();

            std::vector<WW::Span *> spans;

            for (int j = 0; j < COUNT; ++j) {
                WW::Span * span = page_cache.fetchSpan(16 + i * 16);
                EXPECT_NE(span, nullptr);
                EXPECT_EQ(span->count(), 16 + i * 16);
                span->setUsed(1);
                spans.emplace_back(span);
            }

            for (int j = 0; j < COUNT; ++j) {
                spans[j]->setUsed(0);
                page_cache.returnSpan(spans[j]);
            }
        });
    }

    for (int i = 0; i < THREAD_NUM; ++i) {
        threads[i].join();
    }
}