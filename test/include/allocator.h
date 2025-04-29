#pragma once

#include <cstddef>

#include <ThreadCache.h>

/**
 * @brief 分配器
 */
template <typename T>
class allocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    template <typename U>
    class rebind
    {
    public:
        using other = allocator<U>;
    };

private:
    WW::ThreadCache & thread_cache;

public:
    allocator()
        : thread_cache(WW::ThreadCache::getThreadCache())
    {
    }

    allocator(const allocator & other)
        : thread_cache(other.thread_cache)
    {
    }
    
    template <typename U>
    allocator(const allocator<U> & other) 
        : thread_cache(other.thread_cache)
    { // template cannot be default
    };

    ~allocator() = default;

public:
    /**
     * @brief 分配n个元素的内存
     * @param n 元素个数
     * @param hint 会在hint附近分配内存，忽略
     * @return pointer 内存指针
     * @exception std::bad_array_new_length 超出最大尺寸
     * @exception std::bad_alloc 内存分配失败
     */
    pointer allocate(size_type n, const void * hint = nullptr)
    {
        (void)hint;
        if (n > max_size())   // 超出最大尺寸
            throw std::bad_array_new_length();

        if (n == 0)
            return nullptr;

        return static_cast<pointer>(thread_cache.allocate(n * sizeof(T)));
    }

    /**
     * @brief 释放由allocate分配的内存
     * @param ptr 要释放的内存指针
     * @param n 元素个数
     */
    void deallocate(pointer ptr, size_type n)
    {
        if (ptr == nullptr)
            return;
        
        thread_cache.deallocate(ptr, n * sizeof(T));
    }

    /**
     * @brief 构造对象
     * @param ptr 要构造的内存指针
     * @param args 构造函数参数包
     */
    template <typename U, typename... Args>
    void construct(U * ptr, Args&&... args)
    {
        ::new(ptr) U(std::forward<Args>(args)...);
    }

    /**
     * @brief 销毁对象
     * @param ptr 要销毁的内存指针
     */
    template <typename U>
    void destroy(U * ptr)
    {
        ptr->~U();
    }

    size_type max_size() const noexcept
    {
        return std::numeric_limits<std::size_t>::max() / sizeof(value_type);
    }
};
