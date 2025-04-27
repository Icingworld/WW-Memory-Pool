#include "CentralCache.h"

#include <algorithm>

namespace WW
{

CentralCache::CentralCache()
    : page_cache(PageCache::getPageCache())
{
}

CentralCache & CentralCache::getCentralCache()
{
    static CentralCache centralCache;
    return centralCache;
}

CentralCache::~CentralCache()
{
}

FreeObject * CentralCache::fetchRange(std::size_t index, std::size_t count)
{
    // 锁住index对应链表
    std::lock_guard<std::recursive_mutex> lock(spans[index].get_mutex());

    
}

std::size_t CentralCache::indexToSize(std::size_t index) const noexcept
{
    if (index < 128) {
        // 0 ~ 127，8B对齐
        return (index + 1) * 8;
    }
    index -= 128;
    if (index < 56) {
        // 128 ~ 183，每128B对齐
        return 1024 + (index + 1) * 128;
    }
    index -= 56;
    if (index < 24) {
        // 184 ~ 207，每1024B对齐
        return 8192 + (index + 1) * 1024;
    }
    index -= 24;
    if (index < 28) {
        // 208 ~ 235，每8KB对齐
        return 32768 + (index + 1) * 8192;
    }

    // 超出范围
    return 0;
}

Span * CentralCache::getFreeSpan(std::size_t index)
{
    Span * span = spans[index].begin();
    while (span != spans[index].end()) {
        // 查看是否有已经切过的页
        if (span->freelist != nullptr) {
            return span;
        } else {
            span = span->next;
        }
    }

    // 没找到空闲的页段，需要向页缓存申请
    std::lock_guard<std::recursive_mutex> lock(spans[index].get_mutex());
    // 查看这个索引对应的链表管理的内存块是多大的
    std::size_t size = indexToSize(index);
    // 计算出应该申请多少页的页段
    // 最多一次需要512个内存块的空间
    constexpr std::size_t max_nums = 512;
    // 申请的总内存大小
    std::size_t total_size = size * max_nums;
    // 初步计算需要多少页
    std::size_t page_count = total_size / 4096;
    // 如果还有剩余内存，则需要申请额外的一页
    if (total_size % 4096 != 0) {
        page_count += 1;
    }
    // 每个页段最多只有128页
    if (page_count > 128) {
        page_count = 128;
    }

    // 申请页段
    span = page_cache.fetchSpan(page_count);
    // 将页段切成size大小的内存块，挂载到freelist上
    // 计算出内存的起始地址
    void * ptr = reinterpret_cast<void *>(span->page_id << 12);

    // 计算每个内存块的大小并将其挂到 freelist 上
    FreeObject * freelist = nullptr;
    std::size_t block_num = std::min(max_nums, span->page_count * 4096 / size);
    for (std::size_t i = 0; i < block_num; ++i) {
        // 偏移每次一个内存块的大小
        void * block_ptr = static_cast<char *>(ptr) + i * size;

        // 创建 FreeObject 来管理内存块
        FreeObject * free_obj = reinterpret_cast<FreeObject *>(block_ptr);
        
        // 设置这个内存块的 freelist 指针
        free_obj->next = freelist;

        // 将此内存块挂到 freelist 上
        freelist = free_obj;
    }

    // 将 freelist 挂到对应的链表上
    span->freelist = freelist;

    return span;
}

} // namespace WW
