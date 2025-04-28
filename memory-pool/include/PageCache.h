#pragma once

#include <array>
#include <unordered_map>

#include <SpanList.h>

namespace WW
{

/**
 * @brief 页缓存
 */
class PageCache
{
private:
    std::array<SpanList, 128> spans;                    // 页段链表数组
    std::unordered_map<std::size_t, Span *> span_map;   // 页号到页段的映射
    std::recursive_mutex mutex;                         // 页缓存锁

private:
    PageCache();

    PageCache(const PageCache &) = delete;

    PageCache & operator=(const PageCache &) = delete;

public:
    ~PageCache();

public:
    /**
     * @brief 获取页缓存单例
     */
    static PageCache & getPageCache();

    /**
     * @brief 获取指定大小的页段
     * @param page_count 页数
     */
    Span * fetchSpan(std::size_t page_count);

    /**
     * @brief 将页段归还到页缓存
     * @param span 页段
     */
    void returnSpan(Span * span);

    /**
     * @brief 通过内存地址找到所属页段
     */
    Span * FreeObjectToSpan(void * ptr);

private:
    /**
     * @brief 从系统内存中获取指定大小的内存
     */
    void * fetchFromSystem(std::size_t page_count) const noexcept;
};

} // namespace WW
