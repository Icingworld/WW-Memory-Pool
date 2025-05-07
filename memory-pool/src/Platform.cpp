#include "Platform.h"

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#elif defined(__linux__)
#include <cstdlib>
#endif

namespace WW
{

void * Platform::align_malloc(size_type alignment, size_type size)
{
    void * ptr = nullptr;

#if defined(_WIN32) || defined(_WIN64)
    ptr = _aligned_malloc(size, alignment);
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
    _aligned_free(ptr);
#elif defined(__linux__)
    std::free(ptr);
#endif
}

} // namespace WW
