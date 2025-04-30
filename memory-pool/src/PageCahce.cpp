#include "PageCache.h"

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
}

PageCache & PageCache::getPageCache()
{
    static PageCache instance;
    return instance;
}

Span * PageCache::fetchSpan(page_count count)
{
    std::lock_guard<std::recursive_mutex> lock(_Mutex);

    // 如果有空闲页段，直接从链表中直接获取页段
    if (!_Spans[count - 1].empty()) {
        Span & span = _Spans[count - 1].front();
        _Spans[count - 1].pop_front();
        return &span;
    }

    // 没有正好这么大的页段，尝试从更大块内存中切出页段
    for (page_count i = count; i < MAX_PAGE_COUNT; ++i) {
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
            // bigger_span的首页号对应的还是这个页段，所以不需要修改
            // bigger_span的尾页号插入哈希表
            _Span_map[bigger_span.id() + bigger_span.count() - 1] = &bigger_span;
            // new_span的首页号插入哈希表
            _Span_map[new_span->id()] = new_span;
            // 修改哈希表中new_span的尾页号
            _Span_map[new_span->id() + new_span->count() - 1] = new_span;

            return &bigger_span;
        }
    }

    // 没找到更大的页段，直接申请一个最大的页段，然后按照上面的流程重新获取页段
    Span * max_span = new Span();
    void * ptr = fetchFromSystem(MAX_PAGE_COUNT);
    if (ptr == nullptr) {
        return nullptr;
    }
    
    // 计算页号
    max_span->setId(Span::ptrToId(ptr));
    max_span->setCount(MAX_PAGE_COUNT);
    // 插入MAX_PAGE_COUNT页的链表中
    _Spans[MAX_PAGE_COUNT - 1].push_front(max_span);
    // 首页号插入哈希表
    _Span_map[max_span->id()] = max_span;
    // 尾页号插入哈希表
    _Span_map[max_span->id() + max_span->count() - 1] = max_span;

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
        page_id page_id_prev = span->id() - 1;
        auto it = _Span_map.find(page_id_prev);
        if (it == _Span_map.end() || it->second->used() != 0) {
            // 没找到页，或者找到了但正在使用
            break;
        }

        // 判断合并后是否超出上限
        if (span->count() + it->second->count() > MAX_PAGE_COUNT) {
            break;
        }

        // 从链表中删除该空闲页
        _Spans[it->second->count() - 1].erase(it->second);
        // 从哈希表中删除该空闲页的首尾页号
        _Span_map.erase(it->second->id());
        _Span_map.erase(it->second->id() + it->second->count() - 1);

        // 合并页段
        span->setId(it->second->id());
        span->setCount(it->second->count() + span->count());

        // 删除原空闲页
        delete it->second;
    }

    // 向后寻找空闲的页
    while (true) {
        // 寻找下一个页段的首页号
        page_id page_id_next = span->id() + span->count();
        auto it = _Span_map.find(page_id_next);
        if (it == _Span_map.end() || it->second->used() != 0) {
            // 没找到页，或者找到了但正在使用
            break;
        }

        // 判断合并后是否超出上限
        if (span->count() + it->second->count() > MAX_PAGE_COUNT) {
            break;
        }

        // 从链表中删除该空闲页
        _Spans[it->second->count() - 1].erase(it->second);
        // 从哈希表中删除该空闲页的首尾页号
        _Span_map.erase(it->second->id());
        _Span_map.erase(it->second->id() + it->second->count() - 1);

        // 合并页段，首页号不变，只需要调整大小
        span->setCount(it->second->count() + span->count());

        // 删除原空闲页
        delete it->second;
    }

    // 合并完成，插入新的链表
    _Spans[span->count() - 1].push_back(span);
    // 首页号插入哈希表
    _Span_map[span->id()] = span;
    // 尾页号插入哈希表
    _Span_map[span->id() + span->count() - 1] = span;
}

Span * PageCache::FreeObjectToSpan(void * ptr)
{
    page_id id = Span::ptrToId(ptr);

    std::lock_guard<std::recursive_mutex> lock(_Mutex);
    auto it = _Span_map.find(id);
    if (it == _Span_map.end()) {
        return nullptr;
    }

    return it->second;
}

void * PageCache::fetchFromSystem(block_count count) const noexcept
{
    return ::operator new(count * PAGE_SIZE, std::nothrow);
}

} // namespace WW
