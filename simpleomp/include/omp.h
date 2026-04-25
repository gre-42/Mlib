// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

#ifndef _OMP_H
#define _OMP_H

#ifdef __cplusplus
extern "C" {
#endif

// OpenMP API Version (5.0)
#ifndef _OPENMP
#define _OPENMP 201511
#endif

// ============================================================================
// OpenMP Types
// ============================================================================

typedef enum omp_sched_t {
    omp_sched_static = 1,
    omp_sched_dynamic = 2,
    omp_sched_guided = 3,
    omp_sched_auto = 4,
    omp_sched_monotonic = 0x80000000u
} omp_sched_t;

typedef enum omp_proc_bind_t {
    omp_proc_bind_false = 0,
    omp_proc_bind_true = 1,
    omp_proc_bind_master = 2,
    omp_proc_bind_close = 3,
    omp_proc_bind_spread = 4
} omp_proc_bind_t;

typedef enum omp_sync_hint_t {
    omp_sync_hint_none = 0,
    omp_lock_hint_none = omp_sync_hint_none,
    omp_sync_hint_uncontended = 1,
    omp_lock_hint_uncontended = omp_sync_hint_uncontended,
    omp_sync_hint_contended = 2,
    omp_lock_hint_contended = omp_sync_hint_contended,
    omp_sync_hint_nonspeculative = 4,
    omp_lock_hint_nonspeculative = omp_sync_hint_nonspeculative,
    omp_sync_hint_speculative = 8,
    omp_lock_hint_speculative = omp_sync_hint_speculative
} omp_sync_hint_t;

// Lock types (opaque handles)
typedef struct { void* _lk; } omp_lock_t;
typedef struct { void* _lk; int _count; } omp_nest_lock_t;

// ============================================================================
// Thread Management Functions
// ============================================================================

// Set the number of threads to use in subsequent parallel regions
void omp_set_num_threads(int num_threads);

// Get the number of threads in the current parallel region
int omp_get_num_threads(void);

// Get the maximum number of threads that can be used
int omp_get_max_threads(void);

// Get the thread number of the calling thread
int omp_get_thread_num(void);

// Get the number of processors available to the program
int omp_get_num_procs(void);

// Check if currently executing inside a parallel region
int omp_in_parallel(void);

// ============================================================================
// Dynamic Adjustment
// ============================================================================

// Enable/disable dynamic adjustment of the number of threads
void omp_set_dynamic(int dynamic_threads);

// Check if dynamic thread adjustment is enabled
int omp_get_dynamic(void);

// ============================================================================
// Nested Parallelism (Deprecated in OpenMP 5.0, use max_active_levels)
// ============================================================================

// Enable/disable nested parallelism
void omp_set_nested(int nested);

// Check if nested parallelism is enabled
int omp_get_nested(void);

// ============================================================================
// Active Levels
// ============================================================================

// Set the maximum number of nested active parallel regions
void omp_set_max_active_levels(int max_levels);

// Get the maximum number of nested active parallel regions
int omp_get_max_active_levels(void);

// Get the number of nested parallel regions enclosing the task
int omp_get_level(void);

// Get the thread number of the ancestor at the specified level
int omp_get_ancestor_thread_num(int level);

// Get the size of the thread team at the specified level
int omp_get_team_size(int level);

// Get the number of nested active parallel regions
int omp_get_active_level(void);

// ============================================================================
// Scheduling
// ============================================================================

// Set the schedule kind and chunk size for dynamic/guided loops
void omp_set_schedule(omp_sched_t kind, int chunk_size);

// Get the schedule kind and chunk size
void omp_get_schedule(omp_sched_t* kind, int* chunk_size);

// ============================================================================
// Task Management
// ============================================================================

// Check if executing in a final task region
int omp_in_final(void);

// Get the maximum value that can be specified in the priority clause
int omp_get_max_task_priority(void);

// ============================================================================
// Cancellation
// ============================================================================

// Check if cancellation is enabled
int omp_get_cancellation(void);

// ============================================================================
// Lock Functions
// ============================================================================

// Initialize a simple lock
void omp_init_lock(omp_lock_t* lock);

// Initialize a simple lock with hint
void omp_init_lock_with_hint(omp_lock_t* lock, omp_sync_hint_t hint);

// Destroy a simple lock
void omp_destroy_lock(omp_lock_t* lock);

// Acquire a simple lock
void omp_set_lock(omp_lock_t* lock);

// Release a simple lock
void omp_unset_lock(omp_lock_t* lock);

// Try to acquire a simple lock (returns non-zero on success)
int omp_test_lock(omp_lock_t* lock);

// Initialize a nestable lock
void omp_init_nest_lock(omp_nest_lock_t* lock);

// Initialize a nestable lock with hint
void omp_init_nest_lock_with_hint(omp_nest_lock_t* lock, omp_sync_hint_t hint);

// Destroy a nestable lock
void omp_destroy_nest_lock(omp_nest_lock_t* lock);

// Acquire a nestable lock
void omp_set_nest_lock(omp_nest_lock_t* lock);

// Release a nestable lock
void omp_unset_nest_lock(omp_nest_lock_t* lock);

// Try to acquire a nestable lock (returns nesting count on success)
int omp_test_nest_lock(omp_nest_lock_t* lock);

// ============================================================================
// Timing Functions
// ============================================================================

// Get elapsed wall clock time in seconds
double omp_get_wtime(void);

// Get timer precision in seconds
double omp_get_wtick(void);

// ============================================================================
// Processor Binding / Affinity
// ============================================================================

// Get the thread affinity policy
omp_proc_bind_t omp_get_proc_bind(void);

// Get the number of places available
int omp_get_num_places(void);

// Get the number of processors in a place
int omp_get_place_num_procs(int place_num);

// Get the processor IDs in a place
void omp_get_place_proc_ids(int place_num, int* ids);

// Get the place number of the calling thread
int omp_get_place_num(void);

// Get the number of places in the place partition
int omp_get_partition_num_places(void);

// Get the place numbers in the place partition
void omp_get_partition_place_nums(int* place_nums);

// Set the affinity format string
void omp_set_affinity_format(const char* format);

// Get the affinity format string
size_t omp_get_affinity_format(char* buffer, size_t size);

// Display thread affinity information
void omp_display_affinity(const char* format);

// Capture thread affinity information
size_t omp_capture_affinity(char* buffer, size_t size, const char* format);

#ifdef __cplusplus
}
#endif

#endif // _OMP_H
