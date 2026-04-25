// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

#include "platform.h"

#if NCNN_SIMPLEOMP

#include <omp.h>
#include <stdint.h>
#include <map>

namespace ncnn {

// Single construct state for tracking which thread executes the block
// Key is the location pointer (represents the single construct)
static std::map<void*, bool> single_executed;
static ncnn::Mutex single_map_lock;

} // namespace ncnn

#ifdef __cplusplus
extern "C" {
#endif

// Single construct implementation for Clang/LLVM libomp ABI
// Returns 1 if this thread should execute the single block, 0 otherwise
// Only one thread in the team will execute the single block
int32_t __kmpc_single(void* loc, int32_t /*gtid*/)
{
    int num_threads = omp_get_num_threads();

    // Single-threaded case: always execute
    if (num_threads == 1)
    {
        return 1;
    }

    // Use the location pointer as a unique identifier for this single construct
    void* single_id = loc;

    bool should_execute = false;

    ncnn::single_map_lock.lock();

    auto it = ncnn::single_executed.find(single_id);
    if (it == ncnn::single_executed.end())
    {
        // First thread to reach this single construct gets to execute it
        ncnn::single_executed[single_id] = true;
        should_execute = true;
    }

    ncnn::single_map_lock.unlock();

    return should_execute ? 1 : 0;
}

void __kmpc_end_single(void* loc, int32_t /*gtid*/)
{
    int num_threads = omp_get_num_threads();

    // Single-threaded case: nothing to clean up
    if (num_threads == 1)
    {
        return;
    }

    // Clean up the single construct state
    // The thread that executed the single block clears the flag
    void* single_id = loc;

    ncnn::single_map_lock.lock();
    ncnn::single_executed.erase(single_id);
    ncnn::single_map_lock.unlock();
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NCNN_SIMPLEOMP
