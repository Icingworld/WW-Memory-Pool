#pragma once

#include <cstddef>

namespace WW
{

using size_type = std::size_t;                  // 大小类型定义

constexpr size_type PAGE_SIZE = 4096;           // 单页大小
constexpr size_type PAGE_SHIFT = 12;            // 页号计算移位数

constexpr size_type MAX_PAGE_NUM = 128;         // 最大页数

constexpr size_type MAX_ARRAY_SIZE = 208;       // 内存块数组大小

constexpr size_type MAX_MEMORY_SIZE = PAGE_SIZE * MAX_PAGE_NUM / 2;    // 内存池可以管理的最大内存

} // namespace WW
