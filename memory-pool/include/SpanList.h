#pragma once

#include <cstddef>
#include <mutex>

#include <FreeList.h>

namespace WW
{

/**
 * @brief 页段
 * @details 维护一大段连续的内存
 */
class Span
{
public:
    std::size_t page_id;            // 页号
    std::size_t page_count;         // 页数
    Span * prev;                    // 前一个页段
    Span * next;                    // 后一个页段
    FreeList freelist;              // 空闲内存块
    std::size_t used;               // 已使用的内存块数

public:
    Span();

    ~Span();
};

/**
 * @brief 页段链表
 */
class SpanList
{
private:
    Span * head;                    // 虚拟头节点
    std::recursive_mutex mutex;     // 链表锁

public:
    SpanList();

    ~SpanList();

public:
    /**
     * @brief 获取第一个页段
     */
    Span * front() const noexcept;

    /**
     * @brief 获取最后一个页段
     */
    Span * back() const noexcept;

    /**
     * @brief 获取链表头部
     */
    Span * begin() const noexcept;

    /**
     * @brief 获取链表尾部
     */
    Span * end() const noexcept;

    /**
     * @brief 页段链表是否为空
     */
    bool empty() const noexcept;

    /**
     * @brief 将页段插入到头部
     */
    void push_front(Span * span);

    /**
     * @brief 将页段插入到尾部
     */
    void push_back(Span * span);

    /**
     * @brief 从头部删除页段
     */
    void pop_front();

    /**
     * @brief 从尾部删除页段
     */
    void pop_back();

    /**
     * @brief 删除指定页段
     * @param span 要删除的页段
     */
    void erase(Span * span);

    /**
     * @brief 获取链表锁
     */
    std::recursive_mutex & get_mutex();
};

} // namespace WW
