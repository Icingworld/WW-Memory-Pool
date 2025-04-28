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

    // 获取一个合适的空闲页
    Span * span = getFreeSpan(index);
    FreeObject * head = span->freelist.front();
    span->freelist.pop_front();
    ++span->used;

    // 从空闲页中获取空闲内存块
    FreeObject * tail = head;
    for (std::size_t i = 1; i < count; ++i) {
        if (span->freelist.empty()) {
            // 该页段使用完毕，还不够，直接退出
            break;
        }

        // 还有空闲内存块，取出来直接挂上
        FreeObject * new_object = span->freelist.front();
        span->freelist.pop_front();
        tail->next = new_object;
        tail = new_object;

        // 修改使用数量
        ++span->used;
    }

    tail->next = nullptr;

    return head;
}

void CentralCache::returnRange(std::size_t index, FreeObject * free_object)
{
    // 锁住index对应链表
    std::lock_guard<std::recursive_mutex> lock(spans[index].get_mutex());

    // 依次归还空闲内存块
    while (free_object != nullptr) {
        // 查找该内存块属于哪个页段
        Span * span = page_cache.FreeObjectToSpan(free_object);
        // 将内存块加入到该页段的空闲链表中
        FreeObject * next = free_object->next;
        span->freelist.push_front(free_object);
        // 同步页段使用数量
        --span->used;

        if (span->used == 0) {
            // 从链表中删除
            spans[index].erase(span);
            // 归还页缓存
            page_cache.returnSpan(span);
        }

        free_object = next;
    }
}

std::size_t CentralCache::indexToSize(std::size_t index) const noexcept
{
    if (index <= 15) {
        // 8字节对齐
        return (index + 1) * 8;
    } else if (index <= 71) {
        // 16字节对齐
        return 128 + (index - 15) * 16;
    } else if (index <= 127) {
        // 128字节对齐
        return 1024 + (index - 71) * 128;
    } else if (index <= 183) {
        // 1024字节对齐
        return 8192 + (index - 127) * 1024;
    } else if (index <= 207) {
        // 8192字节对齐
        return 65536 + (index - 183) * 8192;
    } else {
        return 0;
    }
}

Span * CentralCache::getFreeSpan(std::size_t index)
{
    Span * span = spans[index].begin();
    while (span != spans[index].end()) {
        // 查看是否有已经切过的页
        if (!span->freelist.empty()) {
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
    std::size_t block_num = span->page_count * 4096 / size;
    for (std::size_t i = 0; i < block_num; ++i) {
        // 偏移每次一个内存块的大小
        void * block_ptr = static_cast<char *>(ptr) + i * size;
        // 创建 FreeObject 来管理内存块
        FreeObject * free_obj = reinterpret_cast<FreeObject *>(block_ptr);
        // 将此内存块挂到 freelist 上
        span->freelist.push_front(free_obj);
    }

    // 将页段挂到链表上
    spans[index].push_back(span);

    return span;
}

} // namespace WW
