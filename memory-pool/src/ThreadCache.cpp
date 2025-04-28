#include "ThreadCache.h"

namespace WW
{

ThreadCache::ThreadCache()
    : central_cache(CentralCache::getCentralCache())
{
}

ThreadCache::~ThreadCache()
{
    // 归还所有内存块
    for (std::size_t i = 0; i < freelists.size(); ++i) {
        if (!freelists[i].empty()) {
            returnToCentralCache(i, freelists[i].size());
        }
    }
}

ThreadCache & ThreadCache::getThreadCache()
{
    static thread_local ThreadCache threadCache;
    return threadCache;
}

void * ThreadCache::allocate(std::size_t size)
{
    if (size == 0) {
        return nullptr;
    }

    if (size > 1024 * 256) {
        // 直接从堆获取
        return ::operator new(size, std::nothrow);
    }

    // 获取对齐后的大小
    std::size_t round_size = roundUp(size);
    // 找到所在的索引
    std::size_t index = sizeToIndex(round_size);

    if (freelists[index].empty()) {
        // 没有这种内存块，需要申请
        fetchFromCentralCache(index);
    }

    // 有这种内存块，取一个出来
    FreeObject * obj = freelists[index].front();
    freelists[index].pop_front();
    return reinterpret_cast<void *>(obj);
}

void ThreadCache::deallocate(void * ptr, std::size_t size)
{
    if (size == 0) {
        return;
    }

    if (size > 1024 * 256) {
        // 从系统释放
        ::operator delete(ptr);
        return;
    }

    // 获取对齐后的大小
    std::size_t round_size = roundUp(size);
    // 找到所在的索引
    std::size_t index = sizeToIndex(round_size);
    // 把内存插入自由表
    FreeObject * obj = reinterpret_cast<FreeObject *>(ptr);
    freelists[index].push_front(obj);

    // 检查是否需要归还给中心缓存
    if (shouldReturn(index)) {
        returnToCentralCache(index, 20);
    }
}

std::size_t ThreadCache::roundUp(std::size_t size) const noexcept
{
    if (size <= 128) {
        return (size + 8 - 1) & ~(8 - 1);
    } else if (size <= 1024) {
        return (size + 16 - 1) & ~(16 - 1);
    } else if (size <= 8192) {
        return (size + 128 - 1) & ~(128 - 1);
    } else if (size <= 65536) {
        return (size + 1024 - 1) & ~(1024 - 1);
    } else {
        return (size + 8192 - 1) & ~(8192 - 1);
    }
}

std::size_t ThreadCache::sizeToIndex(std::size_t size) const noexcept
{
    if (size <= 128) {
        return (size + 8 - 1) / 8 - 1;
    } else if (size <= 1024) {
        return 16 + (size - 128 + 16 - 1) / 16 - 1;
    } else if (size <= 8192) {
        return 16 + 56 + (size - 1024 + 128 - 1) / 128 - 1;
    } else if (size <= 65536) {
        return 16 + 56 + 56 + (size - 8192 + 1024 - 1) / 1024 - 1;
    } else if (size <= 256 * 1024) {
        return 16 + 56 + 56 + 56 + (size - 65536 + 8192 - 1) / 8192 - 1;
    } else {
        return 0;
    }
}

bool ThreadCache::shouldReturn(std::size_t index) const noexcept
{
    // 后续进行动态扩展
    if (freelists[index].size() >= 20) {
        return true;
    }

    return false;
}

void ThreadCache::fetchFromCentralCache(std::size_t index)
{
    // 一次获取20个，后续扩展
    FreeObject * obj = central_cache.fetchRange(index, 20);
    FreeObject * cur = obj;
    while (cur != nullptr) {
        FreeObject * next = cur->next;
        freelists[index].push_front(cur);
        cur = next;
    }
}

void ThreadCache::returnToCentralCache(std::size_t index, std::size_t nums)
{
    // 归还nums个，后续扩展
    FreeObject * obj = freelists[index].front();
    freelists[index].pop_front();
    FreeObject * cur = obj;

    for (std::size_t count = 1; count < nums; ++count) {
        FreeObject * next = freelists[index].front();
        freelists[index].pop_front();
        cur->next = next;
        cur = next;
    }

    central_cache.returnRange(index, obj);
}

} // namespace WW
