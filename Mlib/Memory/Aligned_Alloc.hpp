#pragma once
#include <cstdlib>

// Detect any Windows environment (MSVC, MinGW, MSYS2)
#if defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WRAPPER_OS_WINDOWS 1
#include <malloc.h>
#endif

namespace Mlib {

// Cross-platform aligned allocation
inline void* aligned_alloc(std::size_t alignment, std::size_t size) {
#if defined(WRAPPER_OS_WINDOWS)
    // Works on MSVC, MSYS2, and MinGW using (size, alignment)
    return _aligned_malloc(size, alignment);
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) || defined(__cplusplus) && (__cplusplus >= 201703L)
    // C++17 or C11 standard alignment for Linux/macOS
    return std::aligned_alloc(alignment, size);
#elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200112L) || defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600)
    // Legacy POSIX systems
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return ptr;
#else
    // Fallback for older systems
    return std::malloc(size);
#endif
}

// Cross-platform aligned free
inline void aligned_free(void* ptr) {
    if (!ptr) return;

#if defined(WRAPPER_OS_WINDOWS)
    // Essential for Windows (including MSYS2) to prevent memory corruption crashes
    _aligned_free(ptr);
#else
    // Linux, macOS, and standard fallbacks
    std::free(ptr);
#endif
}

}
