#include "PageCache.h"

#include <cassert>

#include <Platform.h>

namespace WW
{

PageCache::PageCache()
    : _Spans()
    , _Span_map()
    , _Align_pointers()
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
            // 销毁页段
            delete &span;
        }
    }

    // 释放所有对齐指针
    for (void * ptr : _Align_pointers) {
        Platform::align_free(ptr);
    }
}

PageCache & PageCache::getPageCache()
{
    static PageCache instance;
    return instance;
}

Span * PageCache::fetchSpan(size_type pages)
{
    std::lock_guard<std::recursive_mutex> lock(_Mutex);

    // 如果有空闲页段，直接从链表中直接获取页段
    if (!_Spans[pages - 1].empty()) {
        Span & span = _Spans[pages - 1].front();
        _Spans[pages - 1].pop_front();
        return &span;
    }

    // 没有正好这么大的页段，尝试从更大块内存中切出页段
    for (size_type i = pages; i < MAX_PAGE_NUM; ++i) {
        if (!_Spans[i].empty()) {
            // 取出页段
            Span & bigger_span = _Spans[i].front();
            _Spans[i].pop_front();

            // 切分页段，切成i = pages + old_pages的页段

            // 新建一个new_span用于储存后面长old_pages的页段的页段
            Span * new_span = new Span();
            new_span->setId(bigger_span.id() + pages);
            new_span->setCount(bigger_span.count() - pages);

            // bigger_span用于储存前面长count的页段，直接修改页数
            bigger_span.setCount(pages);

            // 新页段插入到对应链表中
            _Spans[new_span->count() - 1].push_front(new_span);

            // 将两个页段的首尾页号储存到哈希表中
            // bigger_span的首页号和尾页号都还在哈希表中，不需要修改

            // 修改new_span对应的的页号
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

    // 记录该对齐指针
    _Align_pointers.emplace_back(ptr);
    
    // max_span用来返回给中心缓存
    max_span->setId(Span::ptrToId(ptr));
    max_span->setCount(pages);
    // 新建一个页段用于储存后面的部分
    Span * new_span = new Span();
    new_span->setId(max_span->id() + pages);
    new_span->setCount(MAX_PAGE_NUM - pages);

    // 新页段插入页段链表中
    _Spans[new_span->count() - 1].push_front(new_span);

    // 页号插入哈希表
    for (size_type first = 0; first < max_span->count(); ++first) {
        _Span_map[max_span->id() + first] = max_span;
    }

    for (size_type first = 0; first < new_span->count(); ++first) {
        _Span_map[new_span->id() + first] = new_span;
    }

    assert(pages == max_span->count());

    return max_span;
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
    _Spans[span->count() - 1].push_front(span);
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

void * PageCache::fetchFromSystem(size_type pages) const noexcept
{
    return Platform::align_malloc(PAGE_SIZE, pages << PAGE_SHIFT);
}

} // namespace WW
