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
    FreeObject * next;  // 下一个空闲内存块

public:
    FreeObject();

    explicit FreeObject(FreeObject * next);

    ~FreeObject();
};

/**
 * @brief 空闲内存块链表
 */
class FreeList
{
private:
    FreeObject * head;      // 链表头部，不是虚拟节点
    std::size_t freesize;   // 空闲内存块数量

public:
    FreeList();

public:
    /**
     * @brief 获取链表头部元素
     */
    FreeObject * front() const noexcept;

    /**
     * @brief 将空闲内存块插入到链表头部
     */
    void push_front(FreeObject * free_object);

    /**
     * @brief 从链表头部取出空闲内存块
     */
    void pop_front();

    /**
     * @brief 链表是否为空
     */
    bool empty() const noexcept;

    /**
     * @brief 清空链表
     */
    void clear();

    /**
     * @brief 获取空闲内存块数量
     */
    std::size_t size() const noexcept;
};

} // namespace WW
