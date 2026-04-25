// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

// Shared barrier state definitions for kmp_barrier.cpp and kmp_cancel.cpp

#pragma once

#include "platform.h"
#include <map>

namespace ncnn {

// Barrier state for a team of threads
struct BarrierState
{
    int num_threads;        // Total number of threads in the team
    int arrived;            // Number of threads that have arrived at the barrier
    int generation;         // Barrier generation counter to handle multiple barrier calls
    Mutex lock;
    ConditionVariable condition;
};

// Global map to store barrier states for different teams
// Key is the barrier location pointer (represents the barrier instance)
extern std::map<void*, BarrierState*> barrier_states;
extern Mutex barrier_map_lock;

} // namespace ncnn
