#pragma once

#include <array>
#include <unordered_map>

#include <SpanList.h>

namespace WW
{

constexpr std::uint8_t MAX_PAGE_COUNT = 128;    // 最大页数

/**
 * @brief 页缓存
 */
class PageCache
{
public:
    using page_id = std::size_t;            // 页号类型
    using block_count = std::uint16_t;      // 内存块数量类型
    using page_count = std::uint8_t;        // 页数类型

private:
    std::array<SpanList, MAX_PAGE_COUNT> _Spans;        // 页段链表数组
    std::unordered_map<page_id, Span *> _Span_map;      // 页号到页段指针的映射
    std::recursive_mutex _Mutex;                        // 页缓存锁

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
     * @param count 页数
     * @return 成功时返回`Span *`，失败时返回`nullptr`
     */
    Span * fetchSpan(page_count count);

    /**
     * @brief 将页段归还到页缓存
     * @param span 页段
     */
    void returnSpan(Span * span);

    /**
     * @brief 通过内存地址找到所属页段
     * @param ptr 内存地址
     * @return 成功找到返回`Span *`，失败时返回`nullptr`
     */
    Span * FreeObjectToSpan(void * ptr);

private:
    /**
     * @brief 从系统内存中获取指定大小的内存
     */
    void * fetchFromSystem(block_count count) const noexcept;
};

} // namespace WW
