#pragma once

#include <PageCache.h>

namespace WW
{

/**
 * @brief 中心缓存
 */
class CentralCache
{
private:
    PageCache & page_cache;                 // 页缓存
    std::array<SpanList, 208> spans;        // 页段链表数组

private:
    CentralCache();

    CentralCache(const CentralCache &) = delete;

    CentralCache & operator=(const CentralCache &) = delete;

public:
    ~CentralCache();

public:
    /**
     * @brief 获取中心缓存单例
     */
    static CentralCache & getCentralCache();

    /**
     * @brief 获取指定大小的空闲内存块
     * @param index 索引
     * @param count 个数
     * @return 空闲内存块链表
     */
    FreeObject * fetchRange(std::size_t index, std::size_t count);

    /**
     * @brief 将空闲内存块归还到中心缓存
     * @param index 索引
     * @param free_object 空闲内存块链表
     */
    void returnRange(std::size_t index, FreeObject * free_object);

private:
    /**
     * @brief 索引转换为块大小
     */
    std::size_t indexToSize(std::size_t index) const noexcept;

    /**
     * @brief 从自由表中获取一个空闲的页段
     * @param index 索引
     */
    Span * getFreeSpan(std::size_t index);
};

} // namespace WW
