// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

#include "platform.h"

#if NCNN_SIMPLEOMP

#include <stdint.h>
#include <map>

namespace ncnn {

// Global map to store mutexes for named critical sections
// Using a simple approach: each unique lock pointer represents a critical section
static std::map<void*, Mutex*> g_critical_locks;
static Mutex g_critical_map_lock;

} // namespace ncnn

#ifdef __cplusplus
extern "C" {
#endif

// Critical section implementation for Clang/LLVM libomp ABI
void __kmpc_critical(void* /*loc*/, int32_t /*gtid*/, void* lck)
{
    ncnn::g_critical_map_lock.lock();

    // Get or create mutex for this critical section
    ncnn::Mutex* mutex = ncnn::g_critical_locks[lck];
    if (!mutex) {
        mutex = new ncnn::Mutex();
        ncnn::g_critical_locks[lck] = mutex;
    }

    ncnn::g_critical_map_lock.unlock();

    // Lock the critical section
    mutex->lock();
}

void __kmpc_end_critical(void* /*loc*/, int32_t /*gtid*/, void* lck)
{
    ncnn::g_critical_map_lock.lock();
    ncnn::Mutex* mutex = ncnn::g_critical_locks[lck];
    ncnn::g_critical_map_lock.unlock();

    // Unlock the critical section
    if (mutex) {
        mutex->unlock();
    }
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NCNN_SIMPLEOMP
