// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

#include "platform.h"

#if NCNN_SIMPLEOMP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Master construct implementation for Clang/LLVM libomp ABI
// Returns 1 if this is the master thread (thread 0), 0 otherwise
int32_t __kmpc_master(void* /*loc*/, int32_t gtid)
{
    // Master thread is always thread 0
    return gtid == 0;
}

void __kmpc_end_master(void* /*loc*/, int32_t /*gtid*/)
{
    // No cleanup needed for master construct
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NCNN_SIMPLEOMP
