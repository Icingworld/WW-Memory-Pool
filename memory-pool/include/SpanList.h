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
    using page_id = std::size_t;        // 页段号使用size_t存储
    using page_count = std::uint8_t;    // 页数范围为1-128，使用uint8_t存储
    using used_type = std::uint16_t;    // 内存块数量范围为0-65535，使用uint16_t存储
    using pointer = Span *;
    using reference = Span &;

private:
    FreeList _Free_list;            // 空闲内存块
    page_id _Page_id;               // 页段号
    pointer _Prev;                  // 前一个页段
    pointer _Next;                  // 后一个页段 
    page_count _Page_count;         // 页数
    used_type _Used;                // 已使用的内存块数

public:
    Span();

    ~Span() = default;

public:
    /**
     * @brief 获取页号
     */
    page_id id() const noexcept;

    /**
     * @brief 设置页号
     */
    void setId(page_id id) noexcept;

    /**
     * @brief 获取页数
     */
    page_count count() const noexcept;

    /**
     * @brief 设置页数
     */
    void setCount(page_count count) noexcept;

    /**
     * @brief 获取上一个页段
     */
    pointer prev() const noexcept;

    /**
     * @brief 设置上一个页段
     */
    void setPrev(pointer prev) noexcept;

    /**
     * @brief 获取下一个页段
     */
    pointer next() const noexcept;

    /**
     * @brief 设置下一个页段
     */
    void setNext(pointer next) noexcept;

    /**
     * @brief 获取空闲内存块数量
     */
    used_type used() const noexcept;

    /**
     * @brief 设置空闲内存块数量
     */
    void setUsed(used_type used) noexcept;
};

/**
 * @brief 页段链表
 * @details 双向链表，持有一个互斥量，用于在中心缓存中的多线程访问
 */
class SpanList
{
public:
    using pointer = Span::pointer;
    using reference = Span::reference;

private:
    pointer _Head;                      // 虚拟头节点
    std::recursive_mutex _Mutex;        // 链表递归锁

public:
    SpanList();

    ~SpanList();

public:
    /**
     * @brief 获取第一个页段
     */
    reference front() noexcept;

    /**
     * @brief 获取最后一个页段
     */
    reference back() noexcept;

    /**
     * @brief 获取链表头部
     */
    pointer begin() noexcept;

    /**
     * @brief 获取链表尾部
     */
    pointer end() noexcept;

    /**
     * @brief 页段链表是否为空
     */
    bool empty() const noexcept;

    /**
     * @brief 将页段插入到头部
     */
    void push_front(pointer span);

    /**
     * @brief 将页段插入到尾部
     */
    void push_back(pointer span);

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
    void erase(pointer span);

    /**
     * @brief 获取链表锁
     */
    std::recursive_mutex & getMutex();
};

} // namespace WW
