#pragma once

#include <Common.h>

namespace WW
{

/**
 * @brief 平台接口
*/
class Platform
{
public:
    /**
     * @brief 从堆中以对齐方法获取内存
     * @param alignment 对齐大小
     * @param size 获取内存大小
    */
    static void * align_malloc(size_type alignment, size_type size);

    /**
     * @brief 释放以对齐方法获取的内存
     * @param ptr 释放内存的指针
    */
    static void align_free(void * ptr);
};

} // namespace WW
