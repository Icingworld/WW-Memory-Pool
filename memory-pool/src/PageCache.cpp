#include "PageCache.h"

#include <Platform.h>
#include <cassert>

namespace WW
{

PageCache::PageCache()
    : _Spans()
    , _Free_span_map()
    , _Busy_span_map()
    , _Align_pointers()
    , _Mutex()
{
}

PageCache::~PageCache()
{
    std::lock_guard<std::mutex> lock(_Mutex);
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
    std::lock_guard<std::mutex> lock(_Mutex);

    // 如果有空闲页段，直接从链表中直接获取页段
    if (!_Spans[pages - 1].empty()) {
        // 从链表中取出页段
        Span & span = _Spans[pages - 1].front();
        _Spans[pages - 1].pop_front();

        // 从空闲映射表中移除页段
        _Free_span_map.erase(span.id());
        _Free_span_map.erase(span.id() + span.count() - 1);

        // 将页段插入到繁忙映射表中
        _Busy_span_map[span.id()] = &span;
        _Busy_span_map[span.id() + span.count() - 1] = &span;

        return &span;
    }

    // 没有正好这么大的页段，尝试从更大块内存中切出页段
    for (size_type i = pages; i < MAX_PAGE_NUM; ++i) {
        if (!_Spans[i].empty()) {
            // 取出页段，该页段页数为i + 1
            Span & bigger_span = _Spans[i].front();
            _Spans[i].pop_front();

            // 切分i + 1页的页段，切成i = (i + 1 - pages) + pages的两个页段

            // 新建一个split_span用于储存后面长pages页的页段
            Span * split_span = new Span();
            split_span->setId(bigger_span.id() + i + 1 - pages);
            split_span->setCount(pages);

            // bigger_span用于储存前面长i + 1 - pages的页段，直接修改页数
            bigger_span.setCount(i + 1 - pages);

            // 前面的页段插入到对应链表中
            _Spans[bigger_span.count() - 1].push_front(&bigger_span);

            // 修改映射表
            // 解除原bigger_span的尾页号映射
            _Free_span_map.erase(bigger_span.id() + bigger_span.count() + split_span->count() - 1);
            // 添加新bigger_span的尾页号映射
            _Free_span_map[bigger_span.id() + bigger_span.count() - 1] = &bigger_span;

            // split_span页号插入繁忙映射表
            _Busy_span_map[split_span->id()] = split_span;
            _Busy_span_map[split_span->id() + split_span->count() - 1] = split_span;

            return split_span;
        }
    }

    // 没找到更大的页段，直接申请一个最大的页段，然后按照上面的流程重新获取页段
    void * ptr = fetchFromSystem(MAX_PAGE_NUM);
    if (ptr == nullptr) {
        return nullptr;
    }

    // 记录该对齐指针
    _Align_pointers.emplace_back(ptr);

    Span * max_span = new Span();

    if (pages == MAX_PAGE_NUM) {
        // 恰好需要最大页数
        max_span->setId(Span::ptrToId(ptr));
        max_span->setCount(MAX_PAGE_NUM);

        // 建立繁忙表映射
        _Busy_span_map[max_span->id()] = max_span;
        _Busy_span_map[max_span->id() + max_span->count() - 1] = max_span;

        return max_span;
    }
    
    // max_span用来储存MAX_PAGE_NUM - pages的页段
    max_span->setId(Span::ptrToId(ptr));
    max_span->setCount(MAX_PAGE_NUM - pages);

    // 新建一个页段用于返回
    Span * split_span = new Span();
    split_span->setId(max_span->id() + MAX_PAGE_NUM - pages);
    split_span->setCount(pages);

    // 新页段插入页段链表中
    _Spans[max_span->count() - 1].push_front(max_span);

    // max_span页号插入空闲映射表
    _Free_span_map[max_span->id()] = max_span;
    _Free_span_map[max_span->id() + max_span->count() - 1] = max_span;

    // split_span页号插入繁忙映射表
    _Busy_span_map[split_span->id()] = split_span;
    _Busy_span_map[split_span->id() + split_span->count() - 1] = split_span;

    return split_span;
}

void PageCache::returnSpan(Span * span)
{
    std::lock_guard<std::mutex> lock(_Mutex);

    // 从繁忙映射表中删除该页段
    _Busy_span_map.erase(span->id());
    _Busy_span_map.erase(span->id() + span->count() - 1);

    // 向前寻找空闲的页
    auto prev_it = _Free_span_map.find(span->id() - 1);
    while (prev_it != _Free_span_map.end()) {
        Span * span_prev = prev_it->second;

        // 判断合并后是否超出上限
        if (span->count() + span_prev->count() > MAX_PAGE_NUM) {
            break;
        }

        // 从链表中删除该空闲页
        _Spans[span_prev->count() - 1].erase(span_prev);

        // 从哈希表中删除该空闲页
        _Free_span_map.erase(span_prev->id());
        _Free_span_map.erase(span_prev->id() + span_prev->count() - 1);

        // 合并页段
        span->setId(span_prev->id());
        span->setCount(span_prev->count() + span->count());

        // 删除原空闲页
        delete span_prev;

        prev_it = _Free_span_map.find(span->id() - 1);
    }

    // 向后寻找空闲的页
    auto next_it = _Free_span_map.find(span->id() + span->count());
    while (next_it != _Free_span_map.end()) {
        Span * span_next = next_it->second;

        // 判断合并后是否超出上限
        if (span->count() + span_next->count() > MAX_PAGE_NUM) {
            break;
        }

        // 从链表中删除该空闲页
        _Spans[span_next->count() - 1].erase(span_next);

        // 从哈希表中删除该空闲页
        _Free_span_map.erase(span_next->id());
        _Free_span_map.erase(span_next->id() + span_next->count() - 1);

        // 合并页段，首页号不变，只需要调整大小
        span->setCount(span_next->count() + span->count());

        // 删除原空闲页
        delete span_next;

        next_it = _Free_span_map.find(span->id() + span->count());
    }

    // 添加新页段的映射
    _Free_span_map[span->id()] = span;
    _Free_span_map[span->id() + span->count() - 1] = span;

    // 合并完成，插入新的链表
    _Spans[span->count() - 1].push_front(span);
}

Span * PageCache::objectToSpan(void * ptr) noexcept
{
    size_type id = Span::ptrToId(ptr);

    std::lock_guard<std::mutex> lock(_Mutex);

    // 寻找首个大于该页号的迭代器
    auto it = _Busy_span_map.upper_bound(id);

    if (it == _Busy_span_map.begin()) {
        return nullptr;
    }

    --it;
    Span * span = it->second;

    if (id >= span->id() + span->count()) {
        // 不在该页段范围内
        return nullptr;
    }

    return span;
}

void * PageCache::fetchFromSystem(size_type pages) const noexcept
{
    return Platform::align_malloc(PAGE_SIZE, pages << PAGE_SHIFT);
}

} // namespace WW
