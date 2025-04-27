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
    span_4->size = 64;
    EXPECT_NE(span_4, nullptr);
    EXPECT_EQ(span_4->page_count, 4);

    // 申请一个24页的页段
    // 此时页缓存中应当有一个100页的页段
    WW::Span * span_24 = page_cache.fetchSpan(24);
    // 模拟切分页段为内存块
    span_24->size = 64;
    EXPECT_NE(span_24, nullptr);
    EXPECT_EQ(span_24->page_count, 24);

    // 申请一个50页的页段
    // 此时页缓存中应当有一个50页的页段
    WW::Span * span_50 = page_cache.fetchSpan(50);
    // 模拟切分页段为内存块
    span_50->size = 64;
    EXPECT_NE(span_50, nullptr);
    EXPECT_EQ(span_50->page_count, 50);

    // 归还一个24页的页段后，页缓存中应当有一个24页的页段和一个50页的页段
    page_cache.returnSpan(span_24);

    // 归还一个4页的页段后，页缓存中应当有一个28页的页段和一个50页的页段
    page_cache.returnSpan(span_4);

    // 归还一个50页的页段后，页缓存中应当有一个128页的页段
    page_cache.returnSpan(span_50);
}