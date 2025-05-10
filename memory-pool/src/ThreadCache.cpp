#include "ThreadCache.h"

#include <Size.h>

namespace WW
{

ThreadCache::ThreadCache()
    : _Free_lists()
{
}

ThreadCache::~ThreadCache()
{
    // 归还所有内存块
    for (size_type i = 0; i < _Free_lists.size(); ++i) {
        if (!_Free_lists[i].empty()) {
            returnToCentralCache(i, _Free_lists[i].size());
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
    size_type round_size = Size::roundUp(size);
    // 找到所在的索引
    size_type index = Size::sizeToIndex(round_size);

    if (_Free_lists[index].empty()) {
        // 没有这种内存块，需要申请
        fetchFromCentralCache(round_size);
    }

    // 有这种内存块，取一个出来
    FreeObject * obj = _Free_lists[index].front();
    _Free_lists[index].pop_front();
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
    size_type round_size = Size::roundUp(size);
    // 找到所在的索引
    size_type index = Size::sizeToIndex(round_size);
    // 把内存插入自由表
    FreeObject * obj = reinterpret_cast<FreeObject *>(ptr);
    _Free_lists[index].push_front(obj);

    // 检查是否需要归还给中心缓存
    if (shouldReturn(index)) {
        returnToCentralCache(index, _Free_lists[index].max_size());
    }
}

bool ThreadCache::shouldReturn(size_type index) const noexcept
{
    // 超过一次申请的最大数量，归还一部分
    if (_Free_lists[index].size() >= _Free_lists[index].max_size() * 2) {
        return true;
    }

    return false;
}

void ThreadCache::fetchFromCentralCache(size_type size) noexcept
{
    // 每次申请按照最大数量申请，并且提升最大数量
    size_type index = Size::sizeToIndex(size);
    size_type count = _Free_lists[index].max_size();
    if (count > MAX_BLOCK_NUM) {
        count = MAX_BLOCK_NUM;
    }

    FreeObject * obj = CentralCache::getCentralCache().fetchRange(size, count);
    FreeObject * cur = obj;
    
    while (cur != nullptr) {
        FreeObject * next = cur->next();
        _Free_lists[index].push_front(cur);
        cur = next;
    }

    // 提升最大数量
    _Free_lists[index].setMax(count + 1);
}

void ThreadCache::returnToCentralCache(size_type index, size_type nums) noexcept
{
    // 取出nums个内存块组成链表
    FreeObject * head = nullptr;
    for (size_type i = 0; i < nums; ++i) {
        FreeObject * obj = _Free_lists[index].front();
        _Free_lists[index].pop_front();
        obj->setNext(head);
        head = obj;
    }

    CentralCache::getCentralCache().returnRange(Size::indexToSize(index), head);
}

} // namespace WW
