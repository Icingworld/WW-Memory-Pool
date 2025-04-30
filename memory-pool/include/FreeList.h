#pragma once

#include <cstddef>

namespace WW
{

/**
 * @brief 空闲内存块
 */
class FreeObject
{
public:
    using pointer = FreeObject *;
    using reference = FreeObject &;

private:
    pointer _Next;  // 下一个空闲内存块

public:
    FreeObject();

    explicit FreeObject(pointer next);

    ~FreeObject() = default;

public:
    /**
     * @brief 获取下一个空闲内存块
     */
    pointer next() const noexcept;

    /**
     * @brief 设置下一个空闲内存块
     */
    void setNext(pointer next) noexcept;
};

/**
 * @brief 空闲内存块链表迭代器
 */
class FreeListIterator
{
public:
    using pointer = FreeObject::pointer;
    using reference = FreeObject::reference;

private:
    pointer _Free_object;       // 空闲内存块指针

public:
    explicit FreeListIterator(pointer free_object) noexcept;

    ~FreeListIterator() = default;

public:
    /**
     * @brief 迭代器是否相等
     */
    bool operator==(const FreeListIterator & other) const noexcept;

    /**
     * @brief 迭代器是否不相等
     */
    bool operator!=(const FreeListIterator & other) const noexcept;

    /**
     * @brief 解引用迭代器
     * @details 对于内存块，解引用直接返回内存块地址
     */
    pointer operator*() noexcept;

    /**
     * @brief 解引用迭代器
     */
    pointer operator->() noexcept;

    /**
     * @brief 向后移动
     */
    FreeListIterator & operator++() noexcept;

    /**
     * @brief 向后移动
     */
    FreeListIterator operator++(int) noexcept;
};

/**
 * @brief 空闲内存块链表
 * @details 单向链表
 */
class FreeList
{
public:
    using size_type = std::uint16_t;    // 内存块数量范围为0-65535，使用uint16_t存储
    using iterator = FreeListIterator;
    using pointer = FreeListIterator::pointer;
    using reference = FreeListIterator::reference;

private:
    pointer _Head;      // 虚拟头节点
    size_type _Size;    // 空闲内存块数量

public:
    FreeList();

    ~FreeList();

public:
    /**
     * @brief 获取链表头部元素
     */
    pointer front() noexcept;

    /**
     * @brief 将空闲内存块插入到链表头部
     */
    void push_front(pointer free_object);

    /**
     * @brief 从链表头部移除内存块
     */
    void pop_front();

    /**
     * @brief 获取链表头部
     */
    iterator begin() noexcept;

    /**
     * @brief 获取链表尾部
     */
    iterator end() noexcept;

    /**
     * @brief 链表是否为空
     */
    bool empty() const noexcept;

    /**
     * @brief 获取空闲内存块数量
     */
    size_type size() const noexcept;
};

} // namespace WW
