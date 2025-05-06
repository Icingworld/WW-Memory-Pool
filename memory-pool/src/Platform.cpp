#include "Platform.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <cstdlib>
#endif

namespace WW
{

void * Platform::align_malloc(size_type alignment, size_type size)
{
    void * ptr = nullptr;

#if defined(_WIN32) || defined(_WIN64)
    ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif defined(__linux__)
    if (posix_memalign(&ptr, alignment, size) != 0) {
        ptr = nullptr;
    }
#endif

    return ptr;
}

void Platform::align_free(void * ptr)
{
#if defined(_WIN32) || defined(_WIN64)
    VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(__linux__)
    std::free(ptr);
#endif
}

} // namespace WW
