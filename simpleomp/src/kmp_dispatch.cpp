// Copyright 2025 Mu-Tsun Tsai
// SPDX-License-Identifier: MIT

// OpenMP dynamic and guided scheduling support
// Implements __kmpc_dispatch_* functions for LLVM libomp ABI

#include "platform.h"

#if NCNN_SIMPLEOMP

#include <omp.h>
#include <stdint.h>
#include <algorithm>
#include <map>
#include <atomic>
#include <string>
#include <cstdlib>
#include <sstream>
#include <iostream>

// Diagnostic logging flag - set to false for production
#define ENABLE_DISPATCH_DEBUG 0

// Forward declare external functions and TLS variables from simpleomp.cpp (defined outside ncnn namespace)
extern ncnn::ThreadLocalStorage tls_thread_num;

namespace ncnn {

// Schedule kinds from LLVM libomp
// Reference: openmp/runtime/src/kmp.h in llvm-project
enum sched_type {
    kmp_sch_static_chunked = 33,   // static schedule with chunk
    kmp_sch_static = 34,           // static schedule (unspecialized)
    kmp_sch_dynamic_chunked = 35,  // dynamic schedule
    kmp_sch_guided_chunked = 36,   // guided schedule
    kmp_sch_runtime = 37,          // runtime schedule (use OMP_SCHEDULE)
    kmp_sch_auto = 38,             // auto schedule
};

// Schedule type modifiers (high bits)
enum sched_type_modifiers {
    kmp_sch_modifier_monotonic = (1 << 29),  // 0x20000000
    kmp_sch_modifier_nonmonotonic = (1 << 30), // 0x40000000
};

// Extract the base schedule type without modifiers
static inline int get_base_schedule_type(int schedule) {
    return schedule & 0xff;  // Lower 8 bits contain the actual schedule type
}

// Runtime schedule configuration (parsed from OMP_SCHEDULE environment variable)
struct RuntimeScheduleConfig {
    int schedule_type;
    int chunk_size;
    bool initialized;

    RuntimeScheduleConfig() : schedule_type(kmp_sch_static), chunk_size(0), initialized(false) {}
};

static RuntimeScheduleConfig g_runtime_schedule;
static Mutex g_runtime_schedule_lock;

// Parse OMP_SCHEDULE environment variable
// Format: "static[,chunk]" or "dynamic[,chunk]" or "guided[,chunk]"
static void parse_omp_schedule() {
    g_runtime_schedule_lock.lock();

    if (g_runtime_schedule.initialized) {
        g_runtime_schedule_lock.unlock();
        return;
    }

    const char* env = std::getenv("OMP_SCHEDULE");
    if (!env) {
        // Default to static schedule
        g_runtime_schedule.schedule_type = kmp_sch_static;
        g_runtime_schedule.chunk_size = 0;
        g_runtime_schedule.initialized = true;
        g_runtime_schedule_lock.unlock();
        return;
    }

    // Parse the schedule type
    std::string schedule_str(env);
    size_t comma_pos = schedule_str.find(',');
    std::string type_str = schedule_str.substr(0, comma_pos);

    // Determine schedule type
    if (type_str == "static") {
        g_runtime_schedule.schedule_type = kmp_sch_static;
    } else if (type_str == "dynamic") {
        g_runtime_schedule.schedule_type = kmp_sch_dynamic_chunked;
    } else if (type_str == "guided") {
        g_runtime_schedule.schedule_type = kmp_sch_guided_chunked;
    } else {
        // Default to static for unknown types
        g_runtime_schedule.schedule_type = kmp_sch_static;
    }

    // Parse chunk size if present
    if (comma_pos != std::string::npos) {
        std::string chunk_str = schedule_str.substr(comma_pos + 1);
        g_runtime_schedule.chunk_size = std::atoi(chunk_str.c_str());
    } else {
        g_runtime_schedule.chunk_size = 0;
    }

    g_runtime_schedule.initialized = true;
    g_runtime_schedule_lock.unlock();
}

// Get runtime schedule configuration
static void get_runtime_schedule(int* schedule_type, int* chunk_size) {
    if (!g_runtime_schedule.initialized) {
        parse_omp_schedule();
    }
    *schedule_type = g_runtime_schedule.schedule_type;
    *chunk_size = g_runtime_schedule.chunk_size;
}

// Dispatch state for a single loop
template<typename T>
struct DispatchState {
    T lower_bound;              // Original loop lower bound
    T upper_bound;              // Original loop upper bound
    T stride;                   // Loop stride (increment)
    T chunk_size;               // Chunk size for scheduling
    int schedule_type;          // Schedule kind
    std::atomic<T> next_iter;   // Next iteration to assign (for dynamic/guided)
    Mutex lock;                 // Lock for thread-safe access
    std::atomic<int> init_generation; // Generation counter to detect reinitialization
    std::atomic<bool> marked_for_deletion; // Flag to indicate this state should be reset on next use

    DispatchState() : next_iter(0), init_generation(0), marked_for_deletion(false) {}
};

// Global storage for dispatch states, keyed by (gtid, location)
// In practice, we use a simple map with pointer as key
static Mutex g_dispatch_map_lock;
static std::map<void*, DispatchState<int32_t>*> g_dispatch_state_map_i32;
static std::map<void*, DispatchState<uint32_t>*> g_dispatch_state_map_u32;
static std::map<void*, DispatchState<int64_t>*> g_dispatch_state_map_i64;
static std::map<void*, DispatchState<uint64_t>*> g_dispatch_state_map_u64;

// Helper function to get or create dispatch state
template<typename T, typename MapType>
static DispatchState<T>* get_dispatch_state(void* loc, MapType& map) {
    g_dispatch_map_lock.lock();

    auto it = map.find(loc);
    DispatchState<T>* state;

    if (it == map.end()) {
        state = new DispatchState<T>();
        map[loc] = state;
    } else {
        state = it->second;
    }

    g_dispatch_map_lock.unlock();
    return state;
}

// Helper function to calculate next chunk for dynamic scheduling
template<typename T>
static bool get_next_dynamic_chunk(DispatchState<T>* state, T* lower, T* upper) {
    T current = state->next_iter.load(std::memory_order_acquire);

    #if ENABLE_DISPATCH_DEBUG
    int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    std::stringstream ss_entry;
    ss_entry << "[DYNAMIC_ENTRY] Thread " << tid << ": Entering, current=" << current
             << ", upper_bound=" << state->upper_bound << "\n";
    std::cout << ss_entry.str() << std::flush;
    #endif

    while (current <= state->upper_bound) {
        T chunk_start = current;
        T chunk_end = std::min(current + state->chunk_size - 1, state->upper_bound);
        T next_value = chunk_end + 1;

        // Try to atomically claim this chunk
        if (state->next_iter.compare_exchange_weak(current, next_value,
                                                     std::memory_order_acq_rel,
                                                     std::memory_order_acquire)) {
            #if ENABLE_DISPATCH_DEBUG
            std::stringstream ss_success;
            ss_success << "[DYNAMIC_SUCCESS] Thread " << tid << ": Got chunk [" << chunk_start
                       << ", " << chunk_end << "], next_iter now=" << next_value << "\n";
            std::cout << ss_success.str() << std::flush;
            #endif

            *lower = chunk_start;
            *upper = chunk_end;
            return true;
        }
        // If CAS failed, current is updated to the new value, retry
    }

    #if ENABLE_DISPATCH_DEBUG
    std::stringstream ss_no_work;
    ss_no_work << "[DYNAMIC_NO_WORK] Thread " << tid << ": Exiting, current=" << current
               << " > upper_bound=" << state->upper_bound << "\n";
    std::cout << ss_no_work.str() << std::flush;
    #endif

    return false; // No more iterations
}

// Helper function to calculate next chunk for guided scheduling
// Guided scheduling uses exponentially decreasing chunk sizes
template<typename T>
static bool get_next_guided_chunk(DispatchState<T>* state, T* lower, T* upper, int num_threads) {
    T current = state->next_iter.load(std::memory_order_acquire);
    int retry_count = 0;

    while (current <= state->upper_bound) {
        // IMPORTANT: Calculate guided_chunk INSIDE the loop
        // because current may be updated by CAS failure
        T remaining = state->upper_bound - current + 1;

        // Guided schedule: chunk_size = max(1, remaining / (2 * num_threads))
        // But not less than the user-specified minimum chunk_size
        T guided_chunk = std::max(remaining / (2 * num_threads), state->chunk_size);
        guided_chunk = std::max(guided_chunk, (T)1);

        T chunk_start = current;
        T chunk_end = std::min(current + guided_chunk - 1, state->upper_bound);
        T next_value = chunk_end + 1;

        #if ENABLE_DISPATCH_DEBUG
        // Log CAS attempts - helps detect livelock
        if (retry_count > 10) {
            int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
            std::stringstream ss;
            ss << "[GUIDED_CAS] Thread " << tid << ": retry_count=" << retry_count
               << ", current=" << current << ", upper_bound=" << state->upper_bound
               << ", guided_chunk=" << guided_chunk << ", next_value=" << next_value << "\n";
            std::cout << ss.str() << std::flush;
        }
        #endif

        // Try to atomically claim this chunk
        if (state->next_iter.compare_exchange_weak(current, next_value,
                                                     std::memory_order_acq_rel,
                                                     std::memory_order_acquire)) {
            #if ENABLE_DISPATCH_DEBUG
            int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
            std::stringstream ss;
            ss << "[GUIDED_SUCCESS] Thread " << tid << ": Got chunk [" << chunk_start
               << ", " << chunk_end << "], next_iter now=" << next_value << "\n";
            std::cout << ss.str() << std::flush;
            #endif

            *lower = chunk_start;
            *upper = chunk_end;
            return true;
        }
        // If CAS failed, current is now updated to the latest value - retry with new current
        retry_count++;
    }

    #if ENABLE_DISPATCH_DEBUG
    // Log when thread exits because no more work
    int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    std::stringstream ss;
    ss << "[GUIDED_NO_WORK] Thread " << tid << ": Exiting, current=" << current
       << " > upper_bound=" << state->upper_bound << ", retry_count=" << retry_count << "\n";
    std::cout << ss.str() << std::flush;
    #endif

    return false; // No more iterations
}

// Thread-local storage for expected generation (to detect when init is called again)
static ThreadLocalStorage tls_expected_generation;

} // namespace ncnn

extern "C" {

// Initialize dispatch state for dynamic/guided scheduling
void __kmpc_dispatch_init_4(void* loc, int32_t gtid, int32_t schedule,
                            int32_t lower, int32_t upper, int32_t stride, int32_t chunk) {
    using namespace ncnn;

    DispatchState<int32_t>* state = get_dispatch_state<int32_t>(loc, g_dispatch_state_map_i32);

    // Use TLS thread_num instead of gtid parameter
    int thread_num = (int)reinterpret_cast<size_t>(tls_thread_num.get());

    #if ENABLE_DISPATCH_DEBUG
    std::stringstream ss_enter;
    ss_enter << "[DISPATCH_INIT_ENTER] Thread " << thread_num << ": loc=" << loc
             << ", state=" << (void*)state << ", current_gen="
             << state->init_generation.load(std::memory_order_relaxed) << "\n";
    std::cout << ss_enter.str() << std::flush;
    #endif

    // Only master thread (thread_num == 0) performs initialization
    // Other threads will directly call dispatch_next which will wait for init
    if (thread_num != 0) {
        return;
    }

    #if ENABLE_DISPATCH_DEBUG
    std::stringstream ss;
    ss << "[DISPATCH_INIT] Thread " << thread_num << ": Acquiring lock for initialization\n";
    std::cout << ss.str() << std::flush;
    #endif

    state->lock.lock();

    // Check if this state was marked for deletion in a previous loop
    // If so, reset it to initial state before reusing
    if (state->marked_for_deletion.load(std::memory_order_acquire)) {
        state->next_iter.store(0, std::memory_order_relaxed);
        state->init_generation.store(0, std::memory_order_relaxed);
        state->marked_for_deletion.store(false, std::memory_order_relaxed);
    }

    int old_gen = state->init_generation.load(std::memory_order_relaxed);

    // Reset for this iteration - set all fields BEFORE incrementing generation
    state->lower_bound = lower;
    state->upper_bound = upper;
    state->stride = stride;

    int base_sched = get_base_schedule_type(schedule);

    // Handle runtime schedule
    if (base_sched == kmp_sch_runtime) {
        int runtime_sched, runtime_chunk;
        get_runtime_schedule(&runtime_sched, &runtime_chunk);
        base_sched = runtime_sched;
        if (chunk == 0 && runtime_chunk > 0) {
            chunk = runtime_chunk;
        }
    }

    state->chunk_size = (chunk > 0) ? chunk : 1;
    state->schedule_type = base_sched;
    state->next_iter.store(lower, std::memory_order_relaxed);

    // Increment generation LAST to signal that initialization is complete
    state->init_generation.store(old_gen + 1, std::memory_order_release);

    #if ENABLE_DISPATCH_DEBUG
    const char* sched_name = (base_sched == kmp_sch_dynamic_chunked) ? "dynamic" :
                             (base_sched == kmp_sch_guided_chunked) ? "guided" : "unknown";
    std::stringstream ss2;
    ss2 << "[DISPATCH_INIT] Thread " << thread_num << ": Initialized loop (gen " << old_gen
        << " -> " << (old_gen + 1) << "), schedule=" << sched_name << ", chunk_size="
        << state->chunk_size << ", range=[" << lower << ", " << upper << "]\n";
    std::cout << ss2.str() << std::flush;
    #endif

    state->lock.unlock();
}

void __kmpc_dispatch_init_4u(void* loc, int32_t gtid, int32_t schedule,
                             uint32_t lower, uint32_t upper, int32_t stride, int32_t chunk) {
    using namespace ncnn;

    DispatchState<uint32_t>* state = get_dispatch_state<uint32_t>(loc, g_dispatch_state_map_u32);

    int thread_num = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    if (thread_num != 0) return;

    state->lock.lock();

    // Reset state if marked for deletion
    if (state->marked_for_deletion.load(std::memory_order_acquire)) {
        state->next_iter.store(0, std::memory_order_relaxed);
        state->init_generation.store(0, std::memory_order_relaxed);
        state->marked_for_deletion.store(false, std::memory_order_relaxed);
    }

    int old_gen = state->init_generation.load(std::memory_order_relaxed);

    state->lower_bound = lower;
    state->upper_bound = upper;
    state->stride = stride;

    int base_sched = get_base_schedule_type(schedule);

    if (base_sched == kmp_sch_runtime) {
        int runtime_sched, runtime_chunk;
        get_runtime_schedule(&runtime_sched, &runtime_chunk);
        base_sched = runtime_sched;
        if (chunk == 0 && runtime_chunk > 0) {
            chunk = runtime_chunk;
        }
    }

    state->chunk_size = (chunk > 0) ? (uint32_t)chunk : 1;
    state->schedule_type = base_sched;
    state->next_iter.store(lower, std::memory_order_relaxed);

    state->init_generation.store(old_gen + 1, std::memory_order_release);

    state->lock.unlock();
}

void __kmpc_dispatch_init_8(void* loc, int32_t gtid, int32_t schedule,
                            int64_t lower, int64_t upper, int64_t stride, int64_t chunk) {
    using namespace ncnn;

    DispatchState<int64_t>* state = get_dispatch_state<int64_t>(loc, g_dispatch_state_map_i64);

    int thread_num = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    if (thread_num != 0) return;

    state->lock.lock();

    // Reset state if marked for deletion
    if (state->marked_for_deletion.load(std::memory_order_acquire)) {
        state->next_iter.store(0, std::memory_order_relaxed);
        state->init_generation.store(0, std::memory_order_relaxed);
        state->marked_for_deletion.store(false, std::memory_order_relaxed);
    }

    int old_gen = state->init_generation.load(std::memory_order_relaxed);

    state->lower_bound = lower;
    state->upper_bound = upper;
    state->stride = stride;

    int base_sched = get_base_schedule_type(schedule);

    if (base_sched == kmp_sch_runtime) {
        int runtime_sched, runtime_chunk;
        get_runtime_schedule(&runtime_sched, &runtime_chunk);
        base_sched = runtime_sched;
        if (chunk == 0 && runtime_chunk > 0) {
            chunk = runtime_chunk;
        }
    }

    state->chunk_size = (chunk > 0) ? chunk : 1;
    state->schedule_type = base_sched;
    state->next_iter.store(lower, std::memory_order_relaxed);

    state->init_generation.store(old_gen + 1, std::memory_order_release);

    state->lock.unlock();
}

void __kmpc_dispatch_init_8u(void* loc, int32_t gtid, int32_t schedule,
                             uint64_t lower, uint64_t upper, int64_t stride, int64_t chunk) {
    using namespace ncnn;

    DispatchState<uint64_t>* state = get_dispatch_state<uint64_t>(loc, g_dispatch_state_map_u64);

    int thread_num = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    if (thread_num != 0) return;

    state->lock.lock();

    // Reset state if marked for deletion
    if (state->marked_for_deletion.load(std::memory_order_acquire)) {
        state->next_iter.store(0, std::memory_order_relaxed);
        state->init_generation.store(0, std::memory_order_relaxed);
        state->marked_for_deletion.store(false, std::memory_order_relaxed);
    }

    int old_gen = state->init_generation.load(std::memory_order_relaxed);

    state->lower_bound = lower;
    state->upper_bound = upper;
    state->stride = stride;

    int base_sched = get_base_schedule_type(schedule);

    if (base_sched == kmp_sch_runtime) {
        int runtime_sched, runtime_chunk;
        get_runtime_schedule(&runtime_sched, &runtime_chunk);
        base_sched = runtime_sched;
        if (chunk == 0 && runtime_chunk > 0) {
            chunk = runtime_chunk;
        }
    }

    state->chunk_size = (chunk > 0) ? (uint64_t)chunk : 1;
    state->schedule_type = base_sched;
    state->next_iter.store(lower, std::memory_order_relaxed);

    state->init_generation.store(old_gen + 1, std::memory_order_release);

    state->lock.unlock();
}

// Get next chunk of iterations for this thread
int __kmpc_dispatch_next_4(void* loc, int32_t gtid, int32_t* last,
                           int32_t* lower, int32_t* upper, int32_t* stride) {
    using namespace ncnn;

    DispatchState<int32_t>* state = get_dispatch_state<int32_t>(loc, g_dispatch_state_map_i32);

    // Check if this is the first call for this loop (generation has changed)
    int expected_gen = (int)reinterpret_cast<size_t>(tls_expected_generation.get());
    int current_gen = state->init_generation.load(std::memory_order_acquire);

    #if ENABLE_DISPATCH_DEBUG
    int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    int wait_count = 0;
    #endif

    // Wait for master thread to complete initialization
    // Generation must be > 0 for a valid loop
    while (current_gen == 0) {
        #if ENABLE_DISPATCH_DEBUG
        // Log excessive waiting - helps detect hypothesis 1 (generation counter race)
        if (wait_count % 1000000 == 0 && wait_count > 0) {
            std::stringstream ss;
            ss << "[DISPATCH_WAIT] Thread " << tid << ": Still waiting for init, wait_count="
               << wait_count << ", current_gen=" << current_gen
               << ", loc=" << loc << ", state=" << (void*)state << "\n";
            std::cout << ss.str() << std::flush;
        }
        wait_count++;
        #endif
        // Spin wait for initialization
        current_gen = state->init_generation.load(std::memory_order_acquire);
    }

    if (expected_gen != current_gen) {
        // New loop started - update expected generation
        // Note: Do NOT wait for marked_for_deletion here - it causes deadlock!
        // The marked_for_deletion flag is only cleared in dispatch_init (which runs on master thread)
        // If we wait here, and master thread is already in barrier waiting for us, we deadlock.
        #if ENABLE_DISPATCH_DEBUG
        std::stringstream ss;
        ss << "[DISPATCH_NEW_LOOP] Thread " << tid << ": New loop detected, expected_gen="
           << expected_gen << " -> current_gen=" << current_gen << "\n";
        std::cout << ss.str() << std::flush;
        #endif
        tls_expected_generation.set(reinterpret_cast<void*>((size_t)current_gen));
    }

    *stride = state->stride;
    int num_threads = omp_get_num_threads();

    bool has_work;

    switch (state->schedule_type) {
        case kmp_sch_dynamic_chunked:
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;

        case kmp_sch_guided_chunked:
            has_work = get_next_guided_chunk(state, lower, upper, num_threads);
            break;

        default:
            // Fallback to dynamic for unknown schedule types
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;
    }

    if (has_work) {
        *last = (*upper >= state->upper_bound) ? 1 : 0;
        return 1;
    }

    #if ENABLE_DISPATCH_DEBUG
    std::stringstream ss;
    ss << "[DISPATCH_EXIT] Thread " << tid << ": No more work available\n";
    std::cout << ss.str() << std::flush;
    #endif

    return 0; // No more work
}

int __kmpc_dispatch_next_4u(void* loc, int32_t /*gtid*/, int32_t* last,
                            uint32_t* lower, uint32_t* upper, int32_t* stride) {
    using namespace ncnn;

    DispatchState<uint32_t>* state = get_dispatch_state<uint32_t>(loc, g_dispatch_state_map_u32);

    // Check if this is the first call for this loop (generation has changed)
    int expected_gen = (int)reinterpret_cast<size_t>(tls_expected_generation.get());
    int current_gen = state->init_generation.load(std::memory_order_acquire);

    #if ENABLE_DISPATCH_DEBUG
    int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    int wait_count = 0;
    #endif

    // Wait for master thread to complete initialization
    // Generation must be > 0 for a valid loop
    while (current_gen == 0) {
        #if ENABLE_DISPATCH_DEBUG
        if (wait_count % 1000000 == 0 && wait_count > 0) {
            std::stringstream ss;
            ss << "[DISPATCH_WAIT] Thread " << tid << ": Still waiting for init, wait_count="
               << wait_count << ", current_gen=" << current_gen
               << ", loc=" << loc << ", state=" << (void*)state << "\n";
            std::cout << ss.str() << std::flush;
        }
        wait_count++;
        #endif
        current_gen = state->init_generation.load(std::memory_order_acquire);
    }

    if (expected_gen != current_gen) {
        // New loop started - update expected generation
        // Note: Do NOT wait for marked_for_deletion here - it causes deadlock!
        #if ENABLE_DISPATCH_DEBUG
        std::stringstream ss;
        ss << "[DISPATCH_NEW_LOOP] Thread " << tid << ": New loop detected, expected_gen="
           << expected_gen << " -> current_gen=" << current_gen << "\n";
        std::cout << ss.str() << std::flush;
        #endif
        tls_expected_generation.set(reinterpret_cast<void*>((size_t)current_gen));
    }

    *stride = state->stride;
    int num_threads = omp_get_num_threads();

    bool has_work;

    switch (state->schedule_type) {
        case kmp_sch_dynamic_chunked:
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;

        case kmp_sch_guided_chunked:
            has_work = get_next_guided_chunk(state, lower, upper, num_threads);
            break;

        default:
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;
    }

    if (has_work) {
        *last = (*upper >= state->upper_bound) ? 1 : 0;
        return 1;
    }

    return 0;
}

int __kmpc_dispatch_next_8(void* loc, int32_t /*gtid*/, int32_t* last,
                           int64_t* lower, int64_t* upper, int64_t* stride) {
    using namespace ncnn;

    DispatchState<int64_t>* state = get_dispatch_state<int64_t>(loc, g_dispatch_state_map_i64);

    // Check if this is the first call for this loop (generation has changed)
    int expected_gen = (int)reinterpret_cast<size_t>(tls_expected_generation.get());
    int current_gen = state->init_generation.load(std::memory_order_acquire);

    #if ENABLE_DISPATCH_DEBUG
    int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    int wait_count = 0;
    #endif

    // Wait for master thread to complete initialization
    // Generation must be > 0 for a valid loop
    while (current_gen == 0) {
        #if ENABLE_DISPATCH_DEBUG
        if (wait_count % 1000000 == 0 && wait_count > 0) {
            std::stringstream ss;
            ss << "[DISPATCH_WAIT] Thread " << tid << ": Still waiting for init, wait_count="
               << wait_count << ", current_gen=" << current_gen
               << ", loc=" << loc << ", state=" << (void*)state << "\n";
            std::cout << ss.str() << std::flush;
        }
        wait_count++;
        #endif
        current_gen = state->init_generation.load(std::memory_order_acquire);
    }

    if (expected_gen != current_gen) {
        // New loop started - update expected generation
        // Note: Do NOT wait for marked_for_deletion here - it causes deadlock!
        #if ENABLE_DISPATCH_DEBUG
        std::stringstream ss;
        ss << "[DISPATCH_NEW_LOOP] Thread " << tid << ": New loop detected, expected_gen="
           << expected_gen << " -> current_gen=" << current_gen << "\n";
        std::cout << ss.str() << std::flush;
        #endif
        tls_expected_generation.set(reinterpret_cast<void*>((size_t)current_gen));
    }

    *stride = state->stride;
    int num_threads = omp_get_num_threads();

    bool has_work;

    switch (state->schedule_type) {
        case kmp_sch_dynamic_chunked:
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;

        case kmp_sch_guided_chunked:
            has_work = get_next_guided_chunk(state, lower, upper, num_threads);
            break;

        default:
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;
    }

    if (has_work) {
        *last = (*upper >= state->upper_bound) ? 1 : 0;
        return 1;
    }

    return 0;
}

int __kmpc_dispatch_next_8u(void* loc, int32_t /*gtid*/, int32_t* last,
                            uint64_t* lower, uint64_t* upper, int64_t* stride) {
    using namespace ncnn;

    DispatchState<uint64_t>* state = get_dispatch_state<uint64_t>(loc, g_dispatch_state_map_u64);

    // Check if this is the first call for this loop (generation has changed)
    int expected_gen = (int)reinterpret_cast<size_t>(tls_expected_generation.get());
    int current_gen = state->init_generation.load(std::memory_order_acquire);

    #if ENABLE_DISPATCH_DEBUG
    int tid = (int)reinterpret_cast<size_t>(tls_thread_num.get());
    int wait_count = 0;
    #endif

    // Wait for master thread to complete initialization
    // Generation must be > 0 for a valid loop
    while (current_gen == 0) {
        #if ENABLE_DISPATCH_DEBUG
        if (wait_count % 1000000 == 0 && wait_count > 0) {
            std::stringstream ss;
            ss << "[DISPATCH_WAIT] Thread " << tid << ": Still waiting for init, wait_count="
               << wait_count << ", current_gen=" << current_gen
               << ", loc=" << loc << ", state=" << (void*)state << "\n";
            std::cout << ss.str() << std::flush;
        }
        wait_count++;
        #endif
        current_gen = state->init_generation.load(std::memory_order_acquire);
    }

    if (expected_gen != current_gen) {
        // New loop started - update expected generation
        // Note: Do NOT wait for marked_for_deletion here - it causes deadlock!
        #if ENABLE_DISPATCH_DEBUG
        std::stringstream ss;
        ss << "[DISPATCH_NEW_LOOP] Thread " << tid << ": New loop detected, expected_gen="
           << expected_gen << " -> current_gen=" << current_gen << "\n";
        std::cout << ss.str() << std::flush;
        #endif
        tls_expected_generation.set(reinterpret_cast<void*>((size_t)current_gen));
    }

    *stride = state->stride;
    int num_threads = omp_get_num_threads();

    bool has_work;

    switch (state->schedule_type) {
        case kmp_sch_dynamic_chunked:
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;

        case kmp_sch_guided_chunked:
            has_work = get_next_guided_chunk(state, lower, upper, num_threads);
            break;

        default:
            has_work = get_next_dynamic_chunk(state, lower, upper);
            break;
    }

    if (has_work) {
        *last = (*upper >= state->upper_bound) ? 1 : 0;
        return 1;
    }

    return 0;
}

// Finalize dispatch (cleanup)
// Note: We use deferred deletion to avoid use-after-free issues
// Instead of deleting the state immediately, we mark it for reset on next use
void __kmpc_dispatch_deinit(void* loc, int32_t gtid) {
    using namespace ncnn;

    // Use TLS thread_num instead of gtid parameter (same as dispatch_init)
    int thread_num = (int)reinterpret_cast<size_t>(tls_thread_num.get());

    // Only cleanup on the master thread to avoid race conditions
    if (thread_num != 0) return;

    g_dispatch_map_lock.lock();

    // Mark states for deletion instead of deleting them immediately
    // This prevents use-after-free when worker threads are still using the state
    auto it_i32 = g_dispatch_state_map_i32.find(loc);
    if (it_i32 != g_dispatch_state_map_i32.end()) {
        it_i32->second->marked_for_deletion.store(true, std::memory_order_release);
    }

    auto it_u32 = g_dispatch_state_map_u32.find(loc);
    if (it_u32 != g_dispatch_state_map_u32.end()) {
        it_u32->second->marked_for_deletion.store(true, std::memory_order_release);
    }

    auto it_i64 = g_dispatch_state_map_i64.find(loc);
    if (it_i64 != g_dispatch_state_map_i64.end()) {
        it_i64->second->marked_for_deletion.store(true, std::memory_order_release);
    }

    auto it_u64 = g_dispatch_state_map_u64.find(loc);
    if (it_u64 != g_dispatch_state_map_u64.end()) {
        it_u64->second->marked_for_deletion.store(true, std::memory_order_release);
    }

    g_dispatch_map_lock.unlock();
}

// Legacy type-specific fini functions (kept for compatibility, not called by Clang)
void __kmpc_dispatch_fini_4(void* loc, int32_t gtid) {
    __kmpc_dispatch_deinit(loc, gtid);
}

void __kmpc_dispatch_fini_4u(void* loc, int32_t gtid) {
    __kmpc_dispatch_deinit(loc, gtid);
}

void __kmpc_dispatch_fini_8(void* loc, int32_t gtid) {
    __kmpc_dispatch_deinit(loc, gtid);
}

void __kmpc_dispatch_fini_8u(void* loc, int32_t gtid) {
    __kmpc_dispatch_deinit(loc, gtid);
}

} // extern "C"

#endif // NCNN_SIMPLEOMP
