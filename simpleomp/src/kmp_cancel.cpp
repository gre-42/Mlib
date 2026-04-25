// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

// OpenMP Cancellation Support
// Implements __kmpc_cancel and __kmpc_cancellationpoint for LLVM libomp ABI

#include "platform.h"

#if NCNN_SIMPLEOMP

#include <map>
#include <atomic>
#include <stdio.h>
#include <omp.h>
#include "kmp_barrier.h"

#define DEBUG_CANCEL 0

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration for __kmpc_barrier (internal runtime function)
void __kmpc_barrier(void* loc, int32_t gtid);

// Cancellation kind constants (from OpenMP specification)
enum {
    cancel_noreq = 0,     // No cancellation request
    cancel_parallel = 1,  // Cancel parallel region
    cancel_loop = 2,      // Cancel loop (for/do)
    cancel_sections = 3,  // Cancel sections
    cancel_taskgroup = 4  // Cancel taskgroup
};

// Global cancellation state
// SimpleOMP doesn't support nested parallelism, so we only need one global flag
// for the currently active parallel region
static std::atomic<int> g_current_cancel_kind(cancel_noreq);

// NOTE: This simple approach works because:
// 1. SimpleOMP doesn't support nested parallel regions
// 2. Parallel regions execute sequentially (one finishes before the next starts)
// 3. Therefore, only one team can be active at any time

/*!
@ingroup CANCELLATION
@param loc source location information
@param gtid global thread id
@param cncl_kind the kind of cancellation (parallel, for, sections, taskgroup)

@return returns non-zero if the encountering thread has to cancel, zero otherwise

The cancel construct activates cancellation of the binding region.
If cancellation is activated, the encountering task will begin executing the
cancellation sequence and the encountering thread will ultimately resume execution
at the end of the canceled region.

IMPORTANT: According to OpenMP specification, #pragma omp cancel includes an
implicit cancellation point. The encountering thread sets the cancellation flag
and then immediately exits the binding region (by returning non-zero).
*/
int __kmpc_cancel(void* loc, int gtid, int cncl_kind)
{
    (void)loc; // Unused in SimpleOMP

    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCEL] Thread %d calling __kmpc_cancel, kind=%d\n", gtid, cncl_kind);
    #endif

    // Check if cancellation is enabled (controlled by OMP_CANCELLATION env var)
    if (!omp_get_cancellation()) {
        #if DEBUG_CANCEL
        fprintf(stderr, "[CANCEL] Thread %d: cancellation disabled\n", gtid);
        #endif
        return 0; // Cancellation is disabled, ignore
    }

    // Get current cancellation state
    int current_kind = g_current_cancel_kind.load(std::memory_order_acquire);

    // If no cancellation is active, or if we're setting a stronger cancellation
    // (cancel_parallel cancels everything, so it takes precedence)
    if (current_kind == cancel_noreq || cncl_kind == cancel_parallel) {
        g_current_cancel_kind.store(cncl_kind, std::memory_order_release);
    }

    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCEL] Thread %d: set cancel_kind=%d, returning 1 (exit immediately)\n", gtid, cncl_kind);
    #endif

    // Return non-zero to make the encountering thread exit immediately
    // #pragma omp cancel includes an implicit cancellation point
    return 1;
}

/*!
@ingroup CANCELLATION
@param loc source location information
@param gtid global thread id
@param cncl_kind the kind of cancellation point (parallel, for, sections, taskgroup)

@return returns non-zero if a matching cancellation request has been flagged

A cancellation point allows a thread to check if cancellation has been requested
for the innermost enclosing region of the specified type.
*/
int __kmpc_cancellationpoint(void* loc, int gtid, int cncl_kind)
{
    (void)loc; // Unused in SimpleOMP

    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCELLATION_POINT] Thread %d checking, kind=%d\n", gtid, cncl_kind);
    #endif

    // Check if cancellation is enabled
    if (!omp_get_cancellation()) {
        return 0; // Cancellation is disabled
    }

    // Get current cancellation state
    int current_kind = g_current_cancel_kind.load(std::memory_order_acquire);

    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCELLATION_POINT] Thread %d: current_cancel_kind=%d\n", gtid, current_kind);
    #endif

    // Check if cancellation matches the requested kind
    // cancel_parallel cancels everything
    if (current_kind == cancel_parallel) {
        #if DEBUG_CANCEL
        fprintf(stderr, "[CANCELLATION_POINT] Thread %d: returning 1 (cancel_parallel)\n", gtid);
        #endif
        return 1;
    }

    // Otherwise, only cancel if the kind matches exactly
    if (current_kind == cncl_kind) {
        #if DEBUG_CANCEL
        fprintf(stderr, "[CANCELLATION_POINT] Thread %d: returning 1 (exact match)\n", gtid);
        #endif
        return 1;
    }

    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCELLATION_POINT] Thread %d: returning 0 (no match)\n", gtid);
    #endif
    return 0;
}

/*!
@ingroup CANCELLATION
Clear the cancellation flag for the current team (called when region ends)
*/
void __kmpc_cancel_clear(void* loc)
{
    (void)loc; // Unused in SimpleOMP

    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCEL_CLEAR] Clearing cancellation state\n");
    #endif

    // Reset the global cancellation flag
    g_current_cancel_kind.store(cancel_noreq, std::memory_order_release);
}

/*!
@ingroup CANCELLATION
@param loc source location information
@param gtid global thread id

Barrier with cancellation point
This is a special barrier that also checks for cancellation

NOTE: According to OpenMP specification, a barrier is an implicit cancellation point.
Three scenarios must be handled:
1. Cancellation active BEFORE reaching barrier → exit immediately, don't wait
2. Cancellation occurs DURING barrier wait → stop waiting, exit
3. No cancellation during barrier → wait until all threads arrive
*/
int __kmpc_cancel_barrier(void* loc, int gtid)
{
    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCEL_BARRIER] Thread %d entering\n", gtid);
    #endif

    // Scenario 1: Check if cancellation is already active BEFORE barrier
    if (omp_get_cancellation()) {
        int current_kind = g_current_cancel_kind.load(std::memory_order_acquire);

        #if DEBUG_CANCEL
        fprintf(stderr, "[CANCEL_BARRIER] Thread %d: current_cancel_kind=%d\n", gtid, current_kind);
        #endif

        // If cancel_parallel is active, exit immediately without entering barrier
        // We need to clean up any existing barrier state for this location
        if (current_kind == cancel_parallel) {
            #if DEBUG_CANCEL
            fprintf(stderr, "[CANCEL_BARRIER] Thread %d: cancellation already active, cleaning up and exiting\n", gtid);
            #endif

            // Clean up barrier state if it exists (in case of cancellation from previous test)
            int thread_num = omp_get_thread_num();
            void* barrier_id = loc;

            if (thread_num == 0) {
                ncnn::barrier_map_lock.lock();
                auto it = ncnn::barrier_states.find(barrier_id);
                if (it != ncnn::barrier_states.end()) {
                    delete it->second;
                    ncnn::barrier_states.erase(it);
                }
                ncnn::barrier_map_lock.unlock();
            }

            return 1;  // Return non-zero to indicate cancellation
        }
    }

    // Scenario 2 & 3: Implement cancellable barrier
    // This is a modified barrier that checks for cancellation during wait

    int num_threads = omp_get_num_threads();

    // Single-threaded case: no barrier needed
    if (num_threads == 1) {
        return 0;
    }

    int thread_num = omp_get_thread_num();
    void* barrier_id = loc;

    ncnn::BarrierState* state = nullptr;

    // Get or create barrier state
    {
        ncnn::barrier_map_lock.lock();
        auto it = ncnn::barrier_states.find(barrier_id);
        if (it == ncnn::barrier_states.end()) {
            state = new ncnn::BarrierState();
            state->num_threads = num_threads;
            state->arrived = 0;
            state->generation = 0;
            ncnn::barrier_states[barrier_id] = state;
        } else {
            state = it->second;
        }
        ncnn::barrier_map_lock.unlock();
    }

    // Synchronize at the barrier with cancellation checking
    {
        state->lock.lock();
        int current_generation = state->generation;
        state->arrived++;

        if (state->arrived == num_threads) {
            // Last thread to arrive: reset and wake up all waiting threads
            state->arrived = 0;
            state->generation++;
            state->condition.broadcast();
            state->lock.unlock();
        } else {
            // Wait for all threads to arrive OR cancellation
            while (current_generation == state->generation) {
                // Check for cancellation during wait
                if (omp_get_cancellation()) {
                    int cancel_kind = g_current_cancel_kind.load(std::memory_order_acquire);
                    if (cancel_kind == cancel_parallel) {
                        #if DEBUG_CANCEL
                        fprintf(stderr, "[CANCEL_BARRIER] Thread %d: cancellation detected during wait\n", gtid);
                        #endif
                        // Decrement arrived count since we're leaving early
                        state->arrived--;
                        // Wake up other threads so they can also check cancellation
                        state->condition.broadcast();
                        state->lock.unlock();
                        return 1;  // Exit with cancellation
                    }
                }
                state->condition.wait(state->lock);
            }
            state->lock.unlock();
        }
    }

    // Clean up barrier state when all threads have passed
    if (thread_num == 0) {
        ncnn::barrier_map_lock.lock();
        state->lock.lock();
        bool should_cleanup = (state->arrived == 0);
        state->lock.unlock();

        if (should_cleanup) {
            ncnn::barrier_states.erase(barrier_id);
            delete state;
        }
        ncnn::barrier_map_lock.unlock();
    }

    #if DEBUG_CANCEL
    fprintf(stderr, "[CANCEL_BARRIER] Thread %d: barrier completed, no cancellation\n", gtid);
    #endif

    return 0;  // Return 0 to indicate no cancellation
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NCNN_SIMPLEOMP
