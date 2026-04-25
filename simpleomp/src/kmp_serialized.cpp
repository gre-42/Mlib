// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

#include "platform.h"

#if NCNN_SIMPLEOMP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Thread-local storage for thread number and current parallel region's num_threads
// These are defined in simpleomp.cpp
extern ncnn::ThreadLocalStorage tls_current_num_threads;
extern ncnn::ThreadLocalStorage tls_thread_num;

// Serialized parallel region functions
// These are used when the if clause evaluates to false

void __kmpc_serialized_parallel(void* /*loc*/, int32_t /*gtid*/)
{
    // NCNN_LOGE("__kmpc_serialized_parallel");
    // Enter a serialized parallel construct
    // This is used when if clause evaluates to false
    // Set to single-threaded mode for this parallel region
    tls_current_num_threads.set(reinterpret_cast<void*>((size_t)1));
    tls_thread_num.set(reinterpret_cast<void*>((size_t)0));
}

void __kmpc_end_serialized_parallel(void* /*loc*/, int32_t /*gtid*/)
{
    // NCNN_LOGE("__kmpc_end_serialized_parallel");
    // Exit a serialized parallel construct
    // Clear current parallel region's num_threads
    tls_current_num_threads.set(reinterpret_cast<void*>((size_t)0));
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NCNN_SIMPLEOMP
