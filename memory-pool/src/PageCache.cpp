#include "PageCache.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace WW
{

PageCache::PageCache()
    : _Spans()
    , _Span_map()
    , _Mutex()
{
}

PageCache::~PageCache()
{
    std::lock_guard<std::recursive_mutex> lock(_Mutex);
    // 释放所有页段
    for (size_type i = 0; i < MAX_PAGE_NUM; ++i) {
        while (!_Spans[i].empty()) {
            Span & span = _Spans[i].front();
            _Spans[i].pop_front();

            // 释放页段管理的内存空间
            void * ptr = Span::idToPtr(span.id());

#if defined(_WIN32) || defined(_WIN64)
            VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(__linux__)
            std::free(ptr);
#endif

            // 销毁页段
            delete(&span);
        }
    }
}

PageCache & PageCache::getPageCache()
{
    static PageCache instance;
    return instance;
}

Span * PageCache::fetchSpan(size_type count)
{
    std::lock_guard<std::recursive_mutex> lock(_Mutex);

    // 如果有空闲页段，直接从链表中直接获取页段
    if (!_Spans[count - 1].empty()) {
        Span & span = _Spans[count - 1].front();
        _Spans[count - 1].pop_front();
        return &span;
    }

    // 没有正好这么大的页段，尝试从更大块内存中切出页段
    for (size_type i = count; i < MAX_PAGE_NUM; ++i) {
        if (!_Spans[i].empty()) {
            // 取出页段
            Span & bigger_span = _Spans[i].front();
            _Spans[i].pop_front();

            // 切分页段，切成i = count + old_count的页段

            // 新建一个new_span用于储存后面长old_count的页段的页段
            Span * new_span = new Span();
            new_span->setId(bigger_span.id() + count);
            new_span->setCount(bigger_span.count() - count);

            // bigger_span用于储存前面长count的页段，直接修改页数
            bigger_span.setCount(count);

            // 新页段插入到对应链表中
            _Spans[new_span->count() - 1].push_back(new_span);

            // 将两个页段的首尾页号储存到哈希表中
            // bigger_span的首页号和尾页号都还在哈希表中，不需要修改

            // new_span的页号插入哈希表
            for (size_type first = 0; first < new_span->count(); ++first) {
                _Span_map[new_span->id() + first] = new_span;
            }

            return &bigger_span;
        }
    }

    // 没找到更大的页段，直接申请一个最大的页段，然后按照上面的流程重新获取页段
    Span * max_span = new Span();
    void * ptr = fetchFromSystem(MAX_PAGE_NUM);
    if (ptr == nullptr) {
        return nullptr;
    }
    
    // 计算页号
    max_span->setId(Span::ptrToId(ptr));
    max_span->setCount(MAX_PAGE_NUM);
    // 插入MAX_PAGE_NUM页的链表中
    _Spans[MAX_PAGE_NUM - 1].push_front(max_span);
    // 页号插入哈希表
    for (size_type first = 0; first < max_span->count(); ++first) {
        _Span_map[max_span->id() + first] = max_span;
    }

    return fetchSpan(count);
}

void PageCache::returnSpan(Span * span)
{
    std::lock_guard<std::recursive_mutex> lock(_Mutex);

    // 向前寻找空闲的页
    while (true) {
        if (span->id() == 0) {
            break;
        }

        // 寻找上一个页段的尾页号
        size_type page_id_prev = span->id() - 1;
        auto it = _Span_map.find(page_id_prev);
        if (it == _Span_map.end() || it->second->used() != 0) {
            // 没找到页，或者找到了但正在使用
            break;
        }

        Span * span_prev = it->second;

        // 判断合并后是否超出上限
        if (span->count() + span_prev->count() > MAX_PAGE_NUM) {
            break;
        }

        // 从链表中删除该空闲页
        _Spans[span_prev->count() - 1].erase(span_prev);

        // 合并页段
        span->setId(span_prev->id());
        span->setCount(span_prev->count() + span->count());

        // 删除原空闲页
        delete span_prev;
    }

    // 向后寻找空闲的页
    while (true) {
        // 寻找下一个页段的首页号
        size_type page_id_next = span->id() + span->count();
        auto it = _Span_map.find(page_id_next);
        if (it == _Span_map.end() || it->second->used() != 0) {
            // 没找到页，或者找到了但正在使用
            break;
        }

        // 判断合并后是否超出上限
        if (span->count() + it->second->count() > MAX_PAGE_NUM) {
            break;
        }

        Span * span_next = it->second;

        // 从链表中删除该空闲页
        _Spans[span_next->count() - 1].erase(span_next);

        // 合并页段，首页号不变，只需要调整大小
        span->setCount(span_next->count() + span->count());

        // 删除原空闲页
        delete span_next;
    }

    // 合并完成，插入新的链表
    _Spans[span->count() - 1].push_back(span);
    // 更新哈希表中的页号
    for (size_type first = 0; first < span->count(); ++first) {
        _Span_map[span->id() + first] = span;
    }
}

Span * PageCache::FreeObjectToSpan(void * ptr)
{
    size_type id = Span::ptrToId(ptr);

    std::lock_guard<std::recursive_mutex> lock(_Mutex);
    auto it = _Span_map.find(id);
    if (it == _Span_map.end()) {
        return nullptr;
    }

    return it->second;
}

void * PageCache::fetchFromSystem(size_type count) const noexcept
{
    void * ptr = nullptr;

#if defined(_WIN32) || defined(_WIN64)
    ptr = VirtualAlloc(nullptr, count * PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif defined(__linux__)
    posix_memalign(&ptr, PAGE_SIZE, count * PAGE_SIZE);
#endif

    return ptr;
}

} // namespace WW
