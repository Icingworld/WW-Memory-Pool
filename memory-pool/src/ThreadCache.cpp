#include "ThreadCache.h"

namespace WW
{

ThreadCache::ThreadCache()
{
}

ThreadCache::~ThreadCache()
{
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
}

std::size_t ThreadCache::roundUp(std::size_t size) const noexcept
{

}

std::size_t ThreadCache::sizeToIndex(std::size_t size) const noexcept
{

}

bool ThreadCache::shouldReturn(std::size_t index) const noexcept
{
    
}

void ThreadCache::fetchFromCentralCache(std::size_t index)
{

}

void ThreadCache::returnToCentralCache(FreeObject * freelist)
{

}

} // namespace WW
