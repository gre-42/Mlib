// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

#include "platform.h"

#if NCNN_SIMPLEOMP

#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include "kmp_barrier.h"

#define DEBUG_BARRIER 0

namespace ncnn {

// Define the global barrier state variables (declared in kmp_barrier.h)
std::map<void*, BarrierState*> barrier_states;
Mutex barrier_map_lock;

} // namespace ncnn

#ifdef __cplusplus
extern "C" {
#endif

// Barrier implementation for Clang/LLVM libomp ABI
// All threads in a team must call this function to synchronize
void __kmpc_barrier(void* loc, int32_t gtid)
{
    int num_threads = omp_get_num_threads();

    // Single-threaded case: no barrier needed
    if (num_threads == 1)
    {
        return;
    }

    int thread_num = omp_get_thread_num();

    // Use the location pointer as a unique identifier for this barrier
    // This allows multiple barriers to coexist in different parallel regions
    void* barrier_id = loc;

    ncnn::BarrierState* state = nullptr;

#if DEBUG_BARRIER
    #pragma omp critical
    {
        fprintf(stderr, "[BARRIER_ENTER] Thread %d: loc=%p\n", thread_num, barrier_id);
    }
#endif

    // Get or create barrier state for this team
    {
        ncnn::barrier_map_lock.lock();

        auto it = ncnn::barrier_states.find(barrier_id);
        if (it == ncnn::barrier_states.end())
        {
            // First thread to arrive creates the barrier state
            state = new ncnn::BarrierState();
            state->num_threads = num_threads;
            state->arrived = 0;
            state->generation = 0;
            ncnn::barrier_states[barrier_id] = state;
#if DEBUG_BARRIER
            #pragma omp critical
            {
                fprintf(stderr, "[BARRIER_CREATE] Thread %d: loc=%p, state=%p\n",
                        thread_num, barrier_id, state);
            }
#endif
        }
        else
        {
            state = it->second;
#if DEBUG_BARRIER
            #pragma omp critical
            {
                fprintf(stderr, "[BARRIER_REUSE] Thread %d: loc=%p, state=%p\n",
                        thread_num, barrier_id, state);
            }
#endif
        }

        ncnn::barrier_map_lock.unlock();
    }

    // Synchronize at the barrier
    {
        state->lock.lock();

        int current_generation = state->generation;
        state->arrived++;

#if DEBUG_BARRIER
        #pragma omp critical
        {
            fprintf(stderr, "[BARRIER_ARRIVE] Thread %d: state=%p, arrived=%d/%d, gen=%d\n",
                    thread_num, state, state->arrived, num_threads, current_generation);
        }
#endif

        if (state->arrived == num_threads)
        {
            // Last thread to arrive: reset counters and wake up all waiting threads
            state->arrived = 0;
            state->generation++;
#if DEBUG_BARRIER
            #pragma omp critical
            {
                fprintf(stderr, "[BARRIER_RELEASE] Thread %d: state=%p, broadcasting, new_gen=%d\n",
                        thread_num, state, state->generation);
            }
#endif
            state->condition.broadcast();
            state->lock.unlock();
        }
        else
        {
            // Wait for all threads to arrive
#if DEBUG_BARRIER
            #pragma omp critical
            {
                fprintf(stderr, "[BARRIER_WAIT] Thread %d: state=%p, waiting for gen %d->%d\n",
                        thread_num, state, current_generation, current_generation + 1);
            }
#endif
            while (current_generation == state->generation)
            {
                state->condition.wait(state->lock);
            }
#if DEBUG_BARRIER
            #pragma omp critical
            {
                fprintf(stderr, "[BARRIER_WAKEUP] Thread %d: state=%p, woke up at gen=%d\n",
                        thread_num, state, state->generation);
            }
#endif
            state->lock.unlock();
        }
    }

    // Note: We do NOT delete the barrier state here to avoid use-after-free issues
    // when the same barrier is reused in a loop. The barrier state will be reused
    // for subsequent barriers at the same location.
    // Memory is cleaned up when the program exits.

#if DEBUG_BARRIER
    #pragma omp critical
    {
        fprintf(stderr, "[BARRIER_EXIT] Thread %d: loc=%p\n", thread_num, barrier_id);
    }
#endif
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NCNN_SIMPLEOMP
