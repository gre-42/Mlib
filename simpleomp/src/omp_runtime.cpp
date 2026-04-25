// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

// OpenMP Runtime Library Functions
// Implements standard omp_* functions defined in omp.h

#include "platform.h"

#if NCNN_SIMPLEOMP

#include "cpu.h"
#include <omp.h>
#include <emscripten.h>
#include <cstring>
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Forward declarations from simpleomp.cpp
// ============================================================================

// These are already implemented in simpleomp.cpp:
// - omp_set_num_threads()
// - omp_get_num_threads()
// - omp_get_max_threads()
// - omp_get_thread_num()
// - omp_set_dynamic()
// - omp_get_dynamic()

// ============================================================================
// Thread Management Functions
// ============================================================================

int omp_get_num_procs(void)
{
    return ncnn::get_cpu_count();
}

int omp_in_parallel(void)
{
    // In SimpleOMP, we're in parallel if thread_num > 0
    // (Thread 0 is the master thread)
    extern int omp_get_thread_num(void);
    extern int omp_get_num_threads(void);
    return omp_get_num_threads() > 1;
}

// ============================================================================
// Nested Parallelism (Deprecated but still supported)
// ============================================================================

static int g_nested_enabled = 0;

void omp_set_nested(int nested)
{
    // SimpleOMP doesn't support nested parallelism yet
    // Just store the value for consistency
    g_nested_enabled = nested;
}

int omp_get_nested(void)
{
    return g_nested_enabled;
}

// ============================================================================
// Active Levels
// ============================================================================

static int g_max_active_levels = 1;

void omp_set_max_active_levels(int max_levels)
{
    if (max_levels >= 0) {
        g_max_active_levels = max_levels;
    }
}

int omp_get_max_active_levels(void)
{
    return g_max_active_levels;
}

int omp_get_level(void)
{
    // SimpleOMP only supports one level of parallelism
    extern int omp_in_parallel(void);
    return omp_in_parallel() ? 1 : 0;
}

int omp_get_ancestor_thread_num(int level)
{
    // For SimpleOMP with single-level parallelism:
    // - level 0: always return 0 (implicit thread)
    // - level 1: return current thread num if in parallel
    extern int omp_get_thread_num(void);
    extern int omp_get_level(void);

    if (level < 0 || level > omp_get_level()) {
        return -1;
    }

    if (level == 0) {
        return 0;
    }

    return omp_get_thread_num();
}

int omp_get_team_size(int level)
{
    // For SimpleOMP with single-level parallelism:
    // - level 0: always 1 (implicit thread)
    // - level 1: return num_threads if in parallel
    extern int omp_get_num_threads(void);
    extern int omp_get_level(void);

    if (level < 0 || level > omp_get_level()) {
        return -1;
    }

    if (level == 0) {
        return 1;
    }

    return omp_get_num_threads();
}

int omp_get_active_level(void)
{
    // Same as omp_get_level() for SimpleOMP
    extern int omp_get_level(void);
    return omp_get_level();
}

// ============================================================================
// Scheduling
// ============================================================================

// Default schedule is static
static omp_sched_t g_schedule_kind = omp_sched_static;
static int g_schedule_chunk = 0;

void omp_set_schedule(omp_sched_t kind, int chunk_size)
{
    g_schedule_kind = kind;
    g_schedule_chunk = chunk_size;
}

void omp_get_schedule(omp_sched_t* kind, int* chunk_size)
{
    if (kind) {
        *kind = g_schedule_kind;
    }
    if (chunk_size) {
        *chunk_size = g_schedule_chunk;
    }
}

// ============================================================================
// Task Management
// ============================================================================

int omp_in_final(void)
{
    // SimpleOMP doesn't support tasks yet
    // Always return 1 (true) to indicate we're in an "included" task
    return 1;
}

int omp_get_max_task_priority(void)
{
    // SimpleOMP doesn't support task priorities
    return 0;
}

// ============================================================================
// Cancellation
// ============================================================================

int omp_get_cancellation(void)
{
    // Check OMP_CANCELLATION environment variable
    // Default is true for SimpleOMP (cancellation enabled by default)
    static int cancellation_enabled = -1; // -1 means not initialized

    if (cancellation_enabled == -1) {
        const char* env = getenv("OMP_CANCELLATION");
        if (env == nullptr) {
            cancellation_enabled = 1; // Default: enabled
        } else {
            // Parse the environment variable (true/false, 1/0)
            if (strcmp(env, "true") == 0 || strcmp(env, "TRUE") == 0 ||
                strcmp(env, "1") == 0) {
                cancellation_enabled = 1;
            } else {
                cancellation_enabled = 0;
            }
        }
    }

    return cancellation_enabled;
}

// ============================================================================
// Lock Functions
// ============================================================================

// SimpleOMP locks are implemented using ncnn::Mutex
// Lock structure contains a pointer to Mutex
// omp_lock_t and omp_nest_lock_t are already defined in omp.h

void omp_init_lock(omp_lock_t* lock)
{
    if (!lock) return;

    // Allocate a new Mutex
    ncnn::Mutex** lock_ptr = (ncnn::Mutex**)&lock->_lk;
    *lock_ptr = new ncnn::Mutex();
}

void omp_init_lock_with_hint(omp_lock_t* lock, omp_sync_hint_t hint)
{
    (void)hint; // Ignore hint in SimpleOMP
    omp_init_lock(lock);
}

void omp_destroy_lock(omp_lock_t* lock)
{
    if (!lock) return;

    ncnn::Mutex** lock_ptr = (ncnn::Mutex**)&lock->_lk;
    if (*lock_ptr) {
        delete *lock_ptr;
        *lock_ptr = nullptr;
    }
}

void omp_set_lock(omp_lock_t* lock)
{
    if (!lock) return;

    ncnn::Mutex** lock_ptr = (ncnn::Mutex**)&lock->_lk;
    if (*lock_ptr) {
        (*lock_ptr)->lock();
    }
}

void omp_unset_lock(omp_lock_t* lock)
{
    if (!lock) return;

    ncnn::Mutex** lock_ptr = (ncnn::Mutex**)&lock->_lk;
    if (*lock_ptr) {
        (*lock_ptr)->unlock();
    }
}

int omp_test_lock(omp_lock_t* lock)
{
    if (!lock) return 0;

    ncnn::Mutex** lock_ptr = (ncnn::Mutex**)&lock->_lk;
    if (*lock_ptr) {
        return (*lock_ptr)->trylock() ? 1 : 0;
    }
    return 0;
}

// Nestable locks - SimpleOMP uses a struct with Mutex + count
struct NestableLock {
    ncnn::Mutex* mutex;
    int owner_thread;
    int count;
};

void omp_init_nest_lock(omp_nest_lock_t* lock)
{
    if (!lock) return;

    NestableLock** nest_lock_ptr = (NestableLock**)&lock->_lk;
    *nest_lock_ptr = new NestableLock();
    (*nest_lock_ptr)->mutex = new ncnn::Mutex();
    (*nest_lock_ptr)->owner_thread = -1;
    (*nest_lock_ptr)->count = 0;
}

void omp_init_nest_lock_with_hint(omp_nest_lock_t* lock, omp_sync_hint_t hint)
{
    (void)hint; // Ignore hint in SimpleOMP
    omp_init_nest_lock(lock);
}

void omp_destroy_nest_lock(omp_nest_lock_t* lock)
{
    if (!lock) return;

    NestableLock** nest_lock_ptr = (NestableLock**)&lock->_lk;
    if (*nest_lock_ptr) {
        delete (*nest_lock_ptr)->mutex;
        delete *nest_lock_ptr;
        *nest_lock_ptr = nullptr;
    }
}

void omp_set_nest_lock(omp_nest_lock_t* lock)
{
    if (!lock) return;

    extern int omp_get_thread_num(void);
    int tid = omp_get_thread_num();

    NestableLock** nest_lock_ptr = (NestableLock**)&lock->_lk;
    NestableLock* nest_lock = *nest_lock_ptr;

    if (!nest_lock) return;

    // If this thread already owns the lock, just increment count
    if (nest_lock->owner_thread == tid) {
        nest_lock->count++;
    } else {
        // Otherwise, acquire the lock
        nest_lock->mutex->lock();
        nest_lock->owner_thread = tid;
        nest_lock->count = 1;
    }
}

void omp_unset_nest_lock(omp_nest_lock_t* lock)
{
    if (!lock) return;

    NestableLock** nest_lock_ptr = (NestableLock**)&lock->_lk;
    NestableLock* nest_lock = *nest_lock_ptr;

    if (!nest_lock) return;

    // Decrement count
    nest_lock->count--;

    // If count reaches 0, release the lock
    if (nest_lock->count == 0) {
        nest_lock->owner_thread = -1;
        nest_lock->mutex->unlock();
    }
}

int omp_test_nest_lock(omp_nest_lock_t* lock)
{
    if (!lock) return 0;

    extern int omp_get_thread_num(void);
    int tid = omp_get_thread_num();

    NestableLock** nest_lock_ptr = (NestableLock**)&lock->_lk;
    NestableLock* nest_lock = *nest_lock_ptr;

    if (!nest_lock) return 0;

    // If this thread already owns the lock, increment count
    if (nest_lock->owner_thread == tid) {
        nest_lock->count++;
        return nest_lock->count;
    }

    // Try to acquire the lock
    if (nest_lock->mutex->trylock()) {
        nest_lock->owner_thread = tid;
        nest_lock->count = 1;
        return 1;
    }

    return 0;
}

// ============================================================================
// Timing Functions
// ============================================================================

double omp_get_wtime(void)
{
    // Use Emscripten's high-resolution timer
    return emscripten_get_now() / 1000.0; // Convert ms to seconds
}

double omp_get_wtick(void)
{
    // Emscripten timer has millisecond precision
    return 0.001; // 1 millisecond
}

// ============================================================================
// Processor Binding / Affinity
// ============================================================================

omp_proc_bind_t omp_get_proc_bind(void)
{
    // SimpleOMP doesn't support processor binding
    // Return false (no binding)
    return omp_proc_bind_false;
}

int omp_get_num_places(void)
{
    // SimpleOMP doesn't support place partitions
    // Return the number of processors as a default
    return ncnn::get_cpu_count();
}

int omp_get_place_num_procs(int place_num)
{
    // Each "place" has 1 processor in SimpleOMP
    (void)place_num;
    return 1;
}

void omp_get_place_proc_ids(int place_num, int* ids)
{
    // Each place corresponds to a processor ID
    if (ids) {
        ids[0] = place_num;
    }
}

int omp_get_place_num(void)
{
    // Return the current thread number as a simple mapping
    extern int omp_get_thread_num(void);
    return omp_get_thread_num();
}

int omp_get_partition_num_places(void)
{
    // SimpleOMP doesn't support place partitions
    return 1;
}

void omp_get_partition_place_nums(int* place_nums)
{
    // Return the current place
    if (place_nums) {
        extern int omp_get_place_num(void);
        place_nums[0] = omp_get_place_num();
    }
}

void omp_set_affinity_format(const char* /*format*/)
{
    // SimpleOMP doesn't support affinity format
    // This is a no-op
}

size_t omp_get_affinity_format(char* buffer, size_t size)
{
    // Return empty format string
    if (buffer && size > 0) {
        buffer[0] = '\0';
    }
    return 0;
}

void omp_display_affinity(const char* /*format*/)
{
    // SimpleOMP doesn't support affinity display
    // This is a no-op
}

size_t omp_capture_affinity(char* buffer, size_t size, const char* /*format*/)
{
    // Return empty affinity string
    if (buffer && size > 0) {
        buffer[0] = '\0';
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // NCNN_SIMPLEOMP
