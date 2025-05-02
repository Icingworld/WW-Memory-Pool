#include "ThreadCache.h"

namespace WW
{

ThreadCache::ThreadCache()
    : _Central_cache(CentralCache::getCentralCache())
{
}

ThreadCache::~ThreadCache()
{
    // 归还所有内存块
    for (size_type i = 0; i < _Freelists.size(); ++i) {
        if (!_Freelists[i].empty()) {
            returnToCentralCache(i, _Freelists[i].size());
        }
    }
}

ThreadCache & ThreadCache::getThreadCache()
{
    static thread_local ThreadCache threadCache;
    return threadCache;
}

void * ThreadCache::allocate(size_type size) noexcept
{
    if (size == 0) {
        return nullptr;
    }

    if (size > MAX_MEMORY_SIZE) {
        // 超出管理范围，直接从堆获取
        return ::operator new(size, std::nothrow);
    }

    // 获取对齐后的大小
    size_type round_size = roundUp(size);
    // 找到所在的索引
    size_type index = sizeToIndex(round_size);

    if (_Freelists[index].empty()) {
        // 没有这种内存块，需要申请
        fetchFromCentralCache(round_size);
    }

    // 有这种内存块，取一个出来
    FreeObject * obj = _Freelists[index].front();
    _Freelists[index].pop_front();
    return reinterpret_cast<void *>(obj);
}

void ThreadCache::deallocate(void * ptr, size_type size) noexcept
{
    if (size == 0) {
        return;
    }

    if (size > MAX_MEMORY_SIZE) {
        // 从系统释放
        ::operator delete(ptr, std::nothrow);
        return;
    }

    // 获取对齐后的大小
    size_type round_size = roundUp(size);
    // 找到所在的索引
    size_type index = sizeToIndex(round_size);
    // 把内存插入自由表
    FreeObject * obj = reinterpret_cast<FreeObject *>(ptr);
    _Freelists[index].push_front(obj);

    // 检查是否需要归还给中心缓存
    if (shouldReturn(index)) {
        returnToCentralCache(index, 20);
    }
}

size_type ThreadCache::roundUp(size_type size) const noexcept
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

size_type ThreadCache::sizeToIndex(size_type size) const noexcept
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

bool ThreadCache::shouldReturn(size_type index) const noexcept
{
    // 后续进行动态扩展
    if (_Freelists[index].size() >= 20) {
        return true;
    }

    return false;
}

void ThreadCache::fetchFromCentralCache(size_type size) noexcept
{
    // 一次获取20个，后续扩展
    FreeObject * obj = _Central_cache.fetchRange(size, 20);
    FreeObject * cur = obj;
    size_type index = sizeToIndex(size);
    while (cur != nullptr) {
        FreeObject * next = cur->next();
        _Freelists[index].push_front(cur);
        cur = next;
    }
}

void ThreadCache::returnToCentralCache(size_type index, size_type nums) noexcept
{
    // 取出nums个内存块组成链表
    FreeObject * head = nullptr;
    for (size_type i = 0; i < nums; ++i) {
        FreeObject * obj = _Freelists[index].front();
        _Freelists[index].pop_front();
        obj->setNext(head);
        head = obj;
    }

    _Central_cache.returnRange(index, head);
}

} // namespace WW
