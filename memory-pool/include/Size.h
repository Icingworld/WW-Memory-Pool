#pragma once

#include <Common.h>

namespace WW
{

/**
 * @brief 大小
 * @details 用于提供索引和大小相互转换的规则
 */
class Size
{
public:
    /**
     * @brief 根据数组索引获取内存块大小
     * @param index 数组索引
     * @return 内存块大小
     */
    static size_type indexToSize(size_type index) noexcept;

    /**
     * @brief 根据内存块大小获取数组索引
     * @param size 内存块大小
     * @return 数组索引
     * @details `size`是对齐之后的大小
     */
    static size_type sizeToIndex(size_type size) noexcept;

    /**
     * @brief 对齐内存块大小
     * @param size 内存块大小
     * @return 对齐后的内存块大小
     */
    static size_type roundUp(size_type size) noexcept;
};

} // namespace WW