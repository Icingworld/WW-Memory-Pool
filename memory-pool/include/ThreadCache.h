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
    CentralCache & central_cache;               // 中心缓存
    std::array<FreeList, 208> freelists;        // 自由表数组

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
     */
    void * allocate(std::size_t size);

    /**
     * @brief 回收内存
     * @param ptr 内存指针
     * @param size 内存大小
     */
    void deallocate(void * ptr, std::size_t size);

private:
    /**
     * @brief 将内存大小向上取整
     */
    std::size_t roundUp(std::size_t size) const noexcept;

    /**
     * @brief 将大小转换为索引
     */
    std::size_t sizeToIndex(std::size_t size) const noexcept;

    /**
     * @brief 判断是否需要归还给中心缓存
     */
    bool shouldReturn(std::size_t index)  const noexcept;

    /**
     * @brief 从中心缓存获取一批内存块
     */
    void fetchFromCentralCache(std::size_t index);

    /**
     * @brief 将一批内存块还给中心缓存
     */
    void returnToCentralCache(std::size_t index, std::size_t nums);
};
    
} // namespace WW
