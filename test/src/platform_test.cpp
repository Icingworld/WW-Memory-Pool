#include <vector>

#include <gtest/gtest.h>
#include <platform.h>

bool is_aligned(void * ptr, std::size_t alignment) {
    return reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0;
}

TEST(PatformTest, AlignedMallocAndFree)
{
    constexpr int COUNT = 1000;
    std::vector<void *> ptrs;

    for (int i = 0; i < COUNT; ++i) {
        void * ptr = WW::Platform::align_malloc(WW::PAGE_SIZE, WW::PAGE_SIZE * WW::MAX_PAGE_NUM);
        ASSERT_NE(ptr, nullptr);
        ASSERT_TRUE(is_aligned(ptr, WW::PAGE_SIZE));
        ptrs.emplace_back(ptr);
    }

    for (int i = 0; i < COUNT; ++i) {
        WW::Platform::align_free(ptrs[i]);
    }
}