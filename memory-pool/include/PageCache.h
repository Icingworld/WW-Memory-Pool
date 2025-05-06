#pragma once

#include <array>
#include <vector>
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
    std::array<SpanList, MAX_PAGE_NUM> _Spans;          // 页段链表数组
    std::unordered_map<size_type, Span *> _Span_map;    // 页号到页段指针的映射
    std::vector<void *> _Align_pointers;                // 对齐指针数组
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
     * @param pages 页数
     * @return 成功时返回`Span *`，失败时返回`nullptr`
     */
    Span * fetchSpan(size_type pages);

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
     * @param pages 页数
     * @return 成功时返回`void *`，失败时返回`nullptr`
     */
    void * fetchFromSystem(size_type pages) const noexcept;
};

} // namespace WW
