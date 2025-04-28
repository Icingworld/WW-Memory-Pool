#include "PageCache.h"

namespace WW
{

PageCache::PageCache()
{
}

PageCache::~PageCache()
{
}

PageCache & PageCache::getPageCache()
{
    static PageCache instance;
    return instance;
}

Span * PageCache::fetchSpan(std::size_t page_count)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    // 尝试从链表中直接获取页段
    if (!spans[page_count - 1].empty()) {
        Span * span = spans[page_count - 1].front();
        spans[page_count - 1].pop_front();
        return span;
    }

    // 尝试从大块内存中切出页段
    for (std::size_t i = page_count; i < spans.size(); ++i) {
        if (!spans[i].empty()) {
            // 取出页段
            Span * bigger_span = spans[i].front();
            spans[i].pop_front();

            // 切分页段，切成page_count + old_count的页段
            // new_span用于储存后面长old_count的页段的页段
            Span * new_span = new Span();
            new_span->page_id = bigger_span->page_id + page_count;
            new_span->page_count = bigger_span->page_count - page_count;

            // bigger_span用于储存前面长page_count的页段，直接修改页数
            bigger_span->page_count = page_count;

            // 新页段插入到对应链表中
            spans[new_span->page_count - 1].push_back(new_span);

            // bigger_span的首页号对应的还是这个页段，所以不需要修改
            // bigger_span的尾页号插入哈希表
            span_map[bigger_span->page_id + bigger_span->page_count - 1] = bigger_span;
            // new_span的首页号插入哈希表
            span_map[new_span->page_id] = new_span;
            // 修改哈希表中new_span的尾页号
            span_map[new_span->page_id + new_span->page_count - 1] = new_span;

            return bigger_span;
        }
    }

    // 没找到更大的页段，直接申请一个最大的页段，然后重新获取页段
    Span * max_span = new Span();
    void * ptr = fetchFromSystem(128);
    if (ptr == nullptr) {
        return nullptr;
    }
    
    // 计算页号
    max_span->page_id = reinterpret_cast<std::uintptr_t>(ptr) >> 12;
    max_span->page_count = 128;
    // 插入128页的链表中
    spans[127].push_front(max_span);
    // 首页号插入哈希表
    span_map[max_span->page_id] = max_span;
    // 尾页号插入哈希表
    span_map[max_span->page_id + max_span->page_count - 1] = max_span;

    return fetchSpan(page_count);
}

void PageCache::returnSpan(Span * span)
{
    std::size_t old_page_count = span->page_count;

    std::lock_guard<std::recursive_mutex> lock(mutex);

    // 向前寻找空闲的页
    while (true) {
        if (span->page_id == 0) {
            break;
        }

        std::size_t page_id_prev = span->page_id - 1;
        auto it = span_map.find(page_id_prev);
        if (it == span_map.end() || it->second->used != 0) {
            // 没找到空闲页
            break;
        }

        // 从链表中删除该空闲页
        spans[it->second->page_count - 1].erase(it->second);
        // 从哈希表中删除该空闲页的首尾页号
        span_map.erase(it->second->page_id);
        span_map.erase(it->second->page_id + it->second->page_count - 1);

        // 合并页段
        span->page_id = it->second->page_id;
        span->page_count += it->second->page_count;

        // 删除原空闲页
        delete it->second;
    }

    // 向后寻找空闲的页
    while (true) {
        std::size_t page_id_next = span->page_id + span->page_count;
        auto it = span_map.find(page_id_next);
        if (it == span_map.end() || it->second->used != 0) {
            // 没找到空闲页
            break;
        }

        // 从链表中删除该空闲页
        spans[it->second->page_count - 1].erase(it->second);
        // 从哈希表中删除该空闲页的首尾页号
        span_map.erase(it->second->page_id);
        span_map.erase(it->second->page_id + it->second->page_count - 1);

        // 合并页段
        span->page_count += it->second->page_count;

        // 删除原空闲页
        delete it->second;
    }

    // 合并完成，插入新的链表
    spans[span->page_count - 1].push_back(span);
    // 首页号插入哈希表
    span_map[span->page_id] = span;
    // 尾页号插入哈希表
    span_map[span->page_id + span->page_count - 1] = span;
}

Span * PageCache::FreeObjectToSpan(void * ptr)
{
    std::uintptr_t page_id = reinterpret_cast<std::uintptr_t>(ptr) >> 12;

    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = span_map.find(page_id);
    if (it == span_map.end()) {
        return nullptr;
    }

    return it->second;
}

void * PageCache::fetchFromSystem(std::size_t page_count) const noexcept
{
    return ::operator new(page_count * 4096, std::nothrow);
}

} // namespace WW
