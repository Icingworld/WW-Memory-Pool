#pragma once

#include <CentralCache.h>

namespace WW
{

/**
 * @brief 线程缓存
 */
class ThreadCache
{
private:
    CentralCache & _Central_cache;                      // 中心缓存
    std::array<FreeList, MAX_ARRAY_SIZE> _Freelists;    // 自由表数组

private:
    ThreadCache();

    ThreadCache(const ThreadCache &) = delete;

    ThreadCache & operator=(const ThreadCache &) = delete;

public:
    ~ThreadCache();

public:
    /**
     * @brief 获取线程缓存单例
     */
    static ThreadCache & getThreadCache();

    /**
     * @brief 申请内存
     * @param size 内存大小
     * @return 成功返回`void *`，失败返回`nullptr`
     */
    void * allocate(size_type size) noexcept;

    /**
     * @brief 回收内存
     * @param ptr 内存指针
     * @param size 内存大小
     */
    void deallocate(void * ptr, size_type size) noexcept;

private:
    /**
     * @brief 将内存大小向上取整
     */
    size_type roundUp(size_type size) const noexcept;

    /**
     * @brief 将大小转换为索引
     */
    size_type sizeToIndex(size_type size) const noexcept;

    /**
     * @brief 判断是否需要归还给中心缓存
     */
    bool shouldReturn(size_type index)  const noexcept;

    /**
     * @brief 从中心缓存获取一批内存块
     */
    void fetchFromCentralCache(size_type size) noexcept;

    /**
     * @brief 将一批内存块还给中心缓存
     * @param index 归还内存所在的索引
     * @param nums 归还的内存数量
     */
    void returnToCentralCache(size_type index, size_type nums) noexcept;
};
    
} // namespace WW
