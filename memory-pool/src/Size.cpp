#include "Size.h"

namespace WW
{

size_type Size::indexToSize(size_type index) noexcept
{
    if (index <= 15) {
        return (index + 1) * 8;
    } else if (index <= 71) {
        return 128 + 16 * (index - 15);  // 144 + 16*(index - 16)
    } else if (index <= 127) {
        return 1024 + 128 * (index - 71); // 1152 + 128*(index - 72)
    } else if (index <= 183) {
        return 8192 + 1024 * (index - 127); // 9216 + 1024*(index - 128)
    } else if (index <= 207) {
        return 65536 + 8192 * (index - 183); // 73728 + 8192*(index - 184)
    } else {
        // 不存在这种情况
        return 0;
    }
}

size_type Size::sizeToIndex(size_type size) noexcept
{
    if (size <= 128) {
        // 8, 16, ..., 128 → 共16类，索引0~15
        return (size + 7) / 8 - 1;
    } else if (size <= 1024) {
        // 144, 160, ..., 1024 → 步长16，共56类，索引16~71
        return 16 + (size - 129) / 16;
    } else if (size <= 8192) {
        // 1152, 1280, ..., 8192 → 步长128，共56类，索引72~127
        return 72 + (size - 1025) / 128;
    } else if (size <= 65536) {
        // 9216, 10240, ..., 65536 → 步长1024，共56类，索引128~183
        return 128 + (size - 8193) / 1024;
    } else if (size <= 262144) {
        // 73728, 81920, ..., 262144 → 步长8192，共24类，索引184~207
        return 184 + (size - 65537) / 8192;
    } else {
        // 不存在这种情况
        return 0;
    }
}

size_type Size::roundUp(size_type size) noexcept
{
    if (size <= 128) {
        // [0, 128]，按照8字节对齐
        return (size + 8 - 1) & ~(8 - 1);
    } else if (size <= 1024) {
        // [129, 1024]，按照16字节对齐
        return (size + 16 - 1) & ~(16 - 1);
    } else if (size <= 8192) {
        // [1025, 8192]，按照128字节对齐
        return (size + 128 - 1) & ~(128 - 1);
    } else if (size <= 65536) {
        // [8193, 65536]，按照1024字节对齐
        return (size + 1024 - 1) & ~(1024 - 1);
    } else {
        // [65537, 262144]，按照8192字节对齐
        return (size + 8192 - 1) & ~(8192 - 1);
    }
}

} // namespace WW