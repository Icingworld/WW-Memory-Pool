#pragma once

#include <mutex>

#include <FreeList.h>

namespace WW
{

constexpr std::size_t PAGE_SIZE = 4096;         // 单页大小
constexpr std::size_t PAGE_SHIFT = 12;          // 页号计算移位数

/**
 * @brief 页段
 * @details 维护一大段连续的内存
 */
class Span
{
public:
    using page_id = std::size_t;        // 页段号使用size_t存储
    using page_count = std::uint8_t;    // 页数范围为1-128，使用uint8_t存储
    using block_count = std::uint16_t;  // 内存块数量范围为0-65535，使用uint16_t存储

private:
    FreeList _Free_list;            // 空闲内存块
    page_id _Page_id;               // 页段号
    Span * _Prev;                   // 前一个页段
    Span * _Next;                   // 后一个页段 
    page_count _Page_count;         // 页数
    block_count _Used;              // 已使用的内存块数

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
    Span * prev() const noexcept;

    /**
     * @brief 设置上一个页段
     */
    void setPrev(Span * prev) noexcept;

    /**
     * @brief 获取下一个页段
     */
    Span * next() const noexcept;

    /**
     * @brief 设置下一个页段
     */
    void setNext(Span * next) noexcept;

    /**
     * @brief 获取空闲内存块数量
     */
    block_count used() const noexcept;

    /**
     * @brief 设置空闲内存块数量
     */
    void setUsed(block_count used) noexcept;

    /**
     * @brief 将内存地址转为页号
     */
    static page_id ptrToId(void * ptr) noexcept;
};

/**
 * @brief 页段链表迭代器
 */
class SpanListIterator
{
private:
    Span * _Span;      // 页段指针

public:
    explicit SpanListIterator(Span * span) noexcept;

    ~SpanListIterator() = default;

public:
    /**
     * @brief 迭代器是否相等
     */
    bool operator==(const SpanListIterator & other) const noexcept;

    /**
     * @brief 迭代器是否不相等
     */
    bool operator!=(const SpanListIterator & other) const noexcept;

    /**
     * @brief 解引用迭代器
     */
    Span & operator*() noexcept;

    /**
     * @brief 解引用迭代器
     */
    Span * operator->() noexcept;

    /**
     * @brief 向后移动
     */
    SpanListIterator & operator++() noexcept;

    /**
     * @brief 向后移动
     */
    SpanListIterator operator++(int) noexcept;

    /**
     * @brief 向前移动
     */
    SpanListIterator & operator--() noexcept;

    /**
     * @brief 向前移动
     */
    SpanListIterator operator--(int) noexcept;
};

/**
 * @brief 页段链表
 * @details 双向链表，持有一个互斥量，用于在中心缓存中的多线程访问
 */
class SpanList
{
public:
    using iterator = SpanListIterator;

private:
    Span * _Head;                       // 虚拟头节点
    std::recursive_mutex _Mutex;        // 链表递归锁

public:
    SpanList();

    ~SpanList();

public:
    /**
     * @brief 获取第一个页段
     */
    Span & front() noexcept;

    /**
     * @brief 获取最后一个页段
     */
    Span & back() noexcept;

    /**
     * @brief 获取链表头部
     */
    iterator begin() noexcept;

    /**
     * @brief 获取链表尾部
     */
    iterator end() noexcept;

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
    std::recursive_mutex & getMutex();
};

} // namespace WW
