#include "CentralCache.h"

#include <algorithm>

namespace WW
{

CentralCache::CentralCache()
    : _Spans()
{
}

CentralCache & CentralCache::getCentralCache()
{
    static CentralCache centralCache;
    return centralCache;
}

FreeObject * CentralCache::fetchRange(size_type size, size_type count)
{
    size_type index = sizeToIndex(size);

    // 锁住index对应链表
    std::unique_lock<std::mutex> lock(_Spans[index].getMutex());

    // 获取一个非空的空闲页
    Span * span = getFreeSpan(size);
    if (span == nullptr) {
        return nullptr;
    }

    FreeObject * head = nullptr;
    for (size_type i = 0; i < count; ++i) {
        // 取出一个内存块，并且插入到新链表的头部
        FreeObject * new_object = span->getFreeList()->front();
        span->getFreeList()->pop_front();
        new_object->setNext(head);
        head = new_object;
        span->setUsed(span->used() + 1);

        if (span->getFreeList()->empty()) {
            // 已经空了，直接返回这么多内存块
            break;
        }
    }

    return head;
}

void CentralCache::returnRange(size_type size, FreeObject * free_object)
{
    size_type index = sizeToIndex(size);

    // 锁住index对应链表
    std::unique_lock<std::mutex> lock(_Spans[index].getMutex());

    // 依次归还空闲内存块
    while (free_object != nullptr) {
        FreeObject * next = free_object->next();

        // 查找该内存块属于哪个页段
        void * ptr = reinterpret_cast<void *>(free_object);

        Span * span = PageCache::getPageCache().objectToSpan(ptr);

        // 没找到则跳过，这种情况不应该出现
        if (span == nullptr) {
            free_object = next;
            continue;
        }

        // 将内存块加入到该页段的空闲链表中
        span->getFreeList()->push_front(free_object);
        // 同步页段使用数量
        span->setUsed(span->used() - 1);

        if (span->used() == 0) {
            // 已经使用完毕，可以从链表中删除
            _Spans[index].erase(span);
            span->getFreeList()->clear();

            // 归还页缓存
            PageCache::getPageCache().returnSpan(span);
        }

        // 向后移动
        free_object = next;
    }
}

size_type CentralCache::sizeToIndex(size_type size) noexcept
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

Span * CentralCache::getFreeSpan(size_type size)
{
    size_type index = sizeToIndex(size);

    for (auto it = _Spans[index].begin(); it != _Spans[index].end(); ++it) {
        // 遍历链表，查看是否有已经切过的页
        if (!it->getFreeList()->empty()) {
            return &*it;
        }
    }

    // 没找到空闲的页段，需要向页缓存申请
    // 计算出应该申请多少页的页段

    // 尝试申请最大数量的内存块，计算申请的总内存大小
    size_type total_size = size * MAX_BLOCK_NUM;
    // 初步计算需要多少页
    size_type page_count = total_size / PAGE_SIZE;
    if (total_size % PAGE_SIZE != 0) {
        page_count += 1;
    }

    // 不能超过最大页数
    if (page_count > MAX_PAGE_NUM) {
        page_count = MAX_PAGE_NUM;
    }

    // 申请页段
    Span * span = PageCache::getPageCache().fetchSpan(page_count);
    if (span == nullptr) {
        return nullptr;
    }
    
    // 将页段切成size大小的内存块，挂载到freelist上
    // 计算出内存的起始地址
    void * ptr = Span::idToPtr(span->id());

    // 计算每个内存块的大小并将其挂到 freelist 上
    size_type block_num = span->count() * PAGE_SIZE / size;
    for (size_type i = 0; i < block_num; ++i) {
        // 偏移每次一个内存块的大小
        void * block_ptr = static_cast<char *>(ptr) + i * size;
        // 创建 FreeObject 来管理内存块
        FreeObject * free_obj = reinterpret_cast<FreeObject *>(block_ptr);
        // 将此内存块挂到 freelist 上
        span->getFreeList()->push_front(free_obj);
    }

    // 将页段挂到链表上
    _Spans[index].push_front(span);

    return span;
}

} // namespace WW
