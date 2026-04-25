# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.6.4] - 2025-01-24

### Fixed
- **Critical: Reduction buffer overflow/use-after-free**: Fixed memory safety bug in `__kmpc_reduce` causing random test failures with "memory access out of bounds" errors
  - **Problem**: Tests would randomly fail with "RuntimeError: memory access out of bounds" in different reduction operations
  - **Root cause**: Thread-local reduction buffer reuse without size checking
    1. TLS buffer allocated with `malloc(reduce_size)` on first reduction
    2. Subsequent reductions with different `reduce_size` would reuse the same buffer
    3. If new `reduce_size` > old buffer size → buffer overflow when `memcpy()` writes beyond allocated memory
    4. Additionally, buffer was freed in `__kmpc_end_reduce` but could be accessed again → use-after-free
  - **Solution**: Store buffer size in header, reallocate when size changes
    - Buffer layout: `[size_t: buffer_size][actual data...]`
    - Check stored size before each reduction operation
    - Reallocate if size mismatch detected
  - **Impact**: Fixes all random "memory access out of bounds" crashes in reduction tests
  - **Files modified**: [src/kmp_reduce.cpp](src/kmp_reduce.cpp)
- **Critical: Barrier use-after-free in loops**: Fixed race condition causing deadlock when `#pragma omp barrier` is reused in loops
  - **Problem**: When a barrier appears inside a loop (e.g., `for (int i = 0; i < 3; i++) { #pragma omp barrier }`), threads would occasionally deadlock on the second or third iteration
  - **Root cause**: Use-after-free + ABA problem in barrier state lifecycle management
    1. Thread 0 completes barrier, becomes last thread to depart, deletes the `BarrierState` object
    2. Thread 0 immediately re-enters the loop and creates a new `BarrierState` at the same location
    3. Memory allocator returns the same address for the new state (ABA problem)
    4. Threads 1-3 are still waking up from `condition.wait()` with a pointer to the old (deleted) state
    5. When Threads 1-3 access the "new" state (same address but different object), they see inconsistent `generation` counters → deadlock
  - **Solution**: Never delete barrier state objects. Instead, reuse the same `BarrierState` object for all subsequent barriers at the same location. Memory is only freed when the program exits.
  - **Impact**: Fixes deadlocks in any code pattern where `#pragma omp barrier` appears inside a loop
  - **Tradeoff**: Small memory leak (one `BarrierState` object per unique barrier location), but guarantees correctness

### Added
- Comprehensive test suites for OpenMP constructs (total: 168 tests across 9 test suites)
  - **[test/specs/atomic.cpp](test/specs/atomic.cpp)** (24 tests): All atomic operations (arithmetic, bitwise, logical, min/max, read/write), different data types, edge cases
  - **[test/specs/barrier.cpp](test/specs/barrier.cpp)** (11 tests): Synchronization semantics, memory visibility, multiple barriers, sequential barrier reuse (revealed use-after-free bug)
  - **[test/specs/critical.cpp](test/specs/critical.cpp)** (13 tests): Named/unnamed critical sections, mutual exclusion, nesting, interaction with other constructs
  - **[test/specs/master.cpp](test/specs/master.cpp)** (18 tests): Master-only execution, interaction with barriers, conditional logic, memory visibility
  - **[test/specs/reduction.cpp](test/specs/reduction.cpp)** (28 tests): All reduction operators (+, -, *, &, |, ^, &&, ||, min, max), multiple variables, schedule/if clause interaction, correctness verification

### Technical Details
- **Modified files**:
  - [src/kmp_reduce.cpp](src/kmp_reduce.cpp): Fixed buffer overflow/use-after-free by storing size in buffer header and reallocating when size changes
  - [src/kmp_barrier.h](src/kmp_barrier.h): Removed `departed` field from `BarrierState` (no longer needed without cleanup logic)
  - [src/kmp_barrier.cpp](src/kmp_barrier.cpp): Removed barrier state deletion logic, added comments explaining the design decision
  - [test/makefile](test/makefile): Increased `INITIAL_MEMORY` from 10MB to 64MB to accommodate comprehensive test suite

## [1.6.3] - 2025-01-23

### Fixed
- **Critical: Dynamic/guided scheduling deadlock**: Fixed deadlock in `__kmpc_dispatch_next_*` functions when worker threads wait for `marked_for_deletion` flag to be cleared
  - **Problem**: Worker threads detected new loop generation and entered spin-wait for `marked_for_deletion` flag, but master thread was already waiting at barrier → deadlock
  - **Root cause**: Incorrect synchronization logic - `marked_for_deletion` is only cleared in `dispatch_init` (master thread), but worker threads tried to wait for it in `dispatch_next`, creating circular dependency
  - **Solution**: Removed the `marked_for_deletion` wait loop from `dispatch_next` functions, allowing worker threads to proceed immediately upon detecting new generation
  - **Impact**: Fixes hang issues when using `schedule(dynamic)` or `schedule(guided)` in consecutive parallel loops

### Added
- Comprehensive test suite for `#pragma omp parallel for` construct ([test/specs/for.cpp](test/specs/for.cpp))
  - 25 test cases covering canonical loop forms, relational operators, schedule types, edge cases
  - Validates correctness of iteration distribution, implicit barriers, thread participation
  - Tests various integer types (signed/unsigned, int/long), loop bounds, and increment patterns
- Comprehensive test suite for `schedule` clause ([test/specs/schedule.cpp](test/specs/schedule.cpp))
  - Tests for `static`, `dynamic`, `guided`, `runtime`, and `auto` schedules
  - Validates chunk size handling, load balancing, and deterministic behavior
  - Covers schedule clause interaction with `if` and `num_threads` clauses

### Technical Details
- **Modified files**:
  - [src/kmp_dispatch.cpp](src/kmp_dispatch.cpp): Removed spin-wait for `marked_for_deletion` in all four `__kmpc_dispatch_next_*` variants, added explanatory comments about deadlock scenario
  - [test/specs/cancel.cpp](test/specs/cancel.cpp): Increased workload in `MultipleCancellationPoints` test for better timing reliability

## [1.6.2] - 2025-01-22

### Fixed
- **Critical: Cancellable barrier deadlock**: Fixed multiple race conditions and deadlocks in `__kmpc_cancel_barrier` implementation
  - **Problem 1 (Barrier deadlock)**: When Thread 0 called `#pragma omp cancel parallel`, it exited immediately, but other threads continued waiting at the barrier expecting all threads to arrive → deadlock
  - **Problem 2 (State pollution)**: Barrier state was not cleaned up when threads exited early due to cancellation, causing subsequent tests to encounter corrupted state (e.g., `arrived = -2`) → hang
  - **Root cause**: Incorrect understanding of OpenMP cancellation semantics - `#pragma omp cancel` includes an implicit cancellation point and causes immediate exit (per [OpenMP specification](https://www.ibm.com/docs/it/xl-c-and-cpp-linux/16.1.0?topic=parallelization-pragma-omp-cancel))
  - **Solution**:
    1. Modified `__kmpc_cancel_barrier` to handle three scenarios:
       - Cancellation active BEFORE barrier → exit immediately without waiting, Thread 0 cleans up barrier state
       - Cancellation occurs DURING barrier wait → decrement arrived count, broadcast to wake other threads, exit
       - No cancellation → normal barrier synchronization
    2. Added cleanup logic for early exit scenario to prevent state pollution between tests

### Technical Details
- **Modified files**:
  - [src/kmp_barrier.h](src/kmp_barrier.h) (NEW): Shared header containing `BarrierState` struct and global variable declarations
  - [src/kmp_barrier.cpp](src/kmp_barrier.cpp): Removed `static` from `barrier_states` and `barrier_map_lock` definitions, include kmp_barrier.h
  - [src/kmp_cancel.cpp](src/kmp_cancel.cpp): Include kmp_barrier.h, implement three-scenario cancellable barrier with cleanup logic
  - [src/platform.h](src/platform.h): Added `#pragma once` guard

## [1.6.1] - 2025-01-21

### Fixed
- **Critical: `num_threads` clause scope isolation**: Fixed incorrect behavior where the `num_threads` clause would persist across multiple parallel regions instead of only affecting the immediately following region
  - **Problem**: `#pragma omp parallel num_threads(4)` followed by `#pragma omp parallel` (without `num_threads`) would incorrectly use 4 threads instead of all available threads
  - **Root cause**: Confused two distinct mechanisms:
    1. `num_threads` clause (temporary, single parallel region scope)
    2. `omp_set_num_threads()` function (persistent, affects all subsequent regions)
  - **Solution**: Implemented proper three-layer TLS variable architecture:
    - `tls_num_threads`: Persistent setting (from `omp_set_num_threads()`)
    - `tls_pushed_num_threads`: Temporary setting (from `num_threads` clause, consumed by next `__kmpc_fork_call`)
    - `tls_current_num_threads`: Active thread count within current parallel region
  - **Impact**: Ensures OpenMP standard-compliant behavior for `num_threads` clause scoping

### Changed
- **Code simplification**: Removed GCC/GOMP code paths (`#else` branch in [src/simpleomp.cpp](src/simpleomp.cpp))
  - SimpleOMP officially only supports Clang/LLVM (Emscripten requirement)
  - Removes ~150 lines of unused GOMP-specific code
  - Improves code maintainability by eliminating dead code paths

### Technical Details
- **Modified files**:
  - [src/simpleomp.cpp](src/simpleomp.cpp):
    - Added `tls_pushed_num_threads` and `tls_current_num_threads` TLS variables
    - Modified `__kmpc_push_num_threads()` to store in `tls_pushed_num_threads`
    - Modified `__kmpc_fork_call()` to consume pushed value and set `tls_current_num_threads`
    - Modified `omp_get_num_threads()` to prioritize `tls_current_num_threads` over persistent setting
    - Clear `tls_current_num_threads` when exiting parallel regions
    - Removed entire `#else` (GCC/GOMP) code block
  - [src/kmp_dispatch.cpp](src/kmp_dispatch.cpp):
    - Changed from directly reading `tls_num_threads` to calling `omp_get_num_threads()`
    - Fixes guided scheduling chunk size calculation to use correct thread count
  - [src/kmp_serialized.cpp](src/kmp_serialized.cpp):
    - Updated to use `tls_current_num_threads` for serialized parallel regions
    - Properly clear on `__kmpc_end_serialized_parallel()`

## [1.6.0] - 2025-01-20

### Added
- Support for OpenMP reduction operations (`#pragma omp reduction`):
  - **Reduction clause**: `reduction(operator:variable)` for parallel accumulation
  - **Arithmetic operators**: `+` (sum), `*` (product), `-` (difference)
  - **Bitwise operators**: `&` (bitwise AND), `|` (bitwise OR), `^` (bitwise XOR)
  - **Logical operators**: `&&` (logical AND), `||` (logical OR)
  - **Comparison operators**: `min` (minimum), `max` (maximum)
  - **Works with**: `#pragma omp parallel for reduction(+:sum)`
  - **Support for `nowait`**: `reduction` clause can be combined with `nowait` to skip implicit barrier
  - Example: [example/src/reduction.cpp](example/src/reduction.cpp)

### Technical Details
- **New file**: [src/kmp_reduce.cpp](src/kmp_reduce.cpp) (~200 lines)
  - Implements `__kmpc_reduce()`, `__kmpc_reduce_nowait()`, `__kmpc_end_reduce()`, `__kmpc_end_reduce_nowait()`
  - Uses atomic operation method (returns 2) to delegate reduction to compiler-generated atomic instructions
  - Thread-local storage maintains per-thread copies of reduction variables
  - Compiler generates code to perform atomic updates on shared reduction variable
  - Supports all standard OpenMP reduction operators
  - Properly handles both blocking (with barrier) and non-blocking (nowait) variants

### Documentation
- Added **Known Limitations** section to README.md:
  - **Nested parallelism not supported**: Documented that SimpleOMP does not support nested `#pragma omp parallel` regions
  - Explained the architectural constraint (single global thread pool)
  - Provided code example showing unsupported usage pattern
  - Listed OpenMP Runtime API functions related to nesting that are provided as stubs
  - Suggested workaround: restructure algorithms to avoid nested parallelism

## [1.5.1] - 2025-01-20

### Fixed
- **Default thread count behavior**: Changed `omp_get_num_threads()` to return the number of CPU cores when not explicitly set, instead of defaulting to 1 (serial execution). This aligns with the OpenMP standard specification where the implementation should default to the number of available processors.
  - Previous behavior: `#pragma omp parallel for` without `num_threads()` used only 1 thread
  - New behavior: Defaults to `navigator.hardwareConcurrency` (number of logical CPU cores)
  - Users can still override with `omp_set_num_threads()` or `num_threads(N)` clause

### Documentation
- **CRITICAL**: Added explicit warnings in README.md and GitHub Release template about the requirement to use `-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency` when linking
  - SimpleOMP creates a worker thread pool sized to match CPU core count at initialization
  - Without this flag, Emscripten's default pthread pool size may be insufficient, causing runtime errors
  - This is a fundamental architectural constraint due to how SimpleOMP pre-allocates worker threads

## [1.5.0] - 2025-01-20

### Added
- Support for OpenMP cancellation constructs:
  - **`#pragma omp cancel`** - Request cancellation of parallel regions or loops
    - Supports cancellation types: `parallel`, `for`, `sections`, `taskgroup`
    - Cancellation propagates to all threads in the same team
    - Returns boolean indicating whether the thread should terminate
  - **`#pragma omp cancellation point`** - Check for cancellation requests
    - Allows threads to respond to cancellation requests at specific points
    - Essential for implementing early termination in parallel algorithms
  - **Environment control**: `OMP_CANCELLATION` environment variable (default: enabled)
  - Example: [example/src/cancel.cpp](example/src/cancel.cpp)
- OpenMP Runtime Library API header file:
  - **New header**: [include/omp.h](include/omp.h) - Standard OpenMP API declarations (OpenMP 5.0 compliant)
  - **New implementation**: [src/omp_runtime.cpp](src/omp_runtime.cpp)
  - **Thread management**: `omp_set_num_threads()`, `omp_get_num_threads()`, `omp_get_thread_num()`, `omp_get_num_procs()`, etc.
  - **Lock mechanisms**: Full implementation of `omp_lock_t` and `omp_nest_lock_t` using `ncnn::Mutex`
  - **Timing functions**: `omp_get_wtime()`, `omp_get_wtick()` using Emscripten high-resolution timers
  - **Schedule control**: `omp_set_schedule()`, `omp_get_schedule()`
  - **Hierarchy queries**: `omp_get_level()`, `omp_get_ancestor_thread_num()`, `omp_get_team_size()`, etc.
  - **Stub implementations**: Specification-compliant stubs for unsupported features (affinity, etc.)
  - Example: [example/src/locks.cpp](example/src/locks.cpp)
- Data-sharing clauses demonstration:
  - **New example**: [example/src/data_sharing.cpp](example/src/data_sharing.cpp)
  - Demonstrates `private`, `shared`, `firstprivate`, `lastprivate` clauses
  - Shows how Clang handles variable scoping at compile-time
  - Educational example explaining compiler-level vs runtime-level features

### Technical Details
- **New file**: [src/kmp_cancel.cpp](src/kmp_cancel.cpp) (~150 lines)
  - Implements `__kmpc_cancel()`, `__kmpc_cancellation_point()`, `__kmpc_cancel_barrier()`
  - Uses global map to track cancellation state per team
  - Thread-safe cancellation flag management with atomic operations
  - Supports all OpenMP cancellation construct types via bitmask flags
  - Environment variable control: reads `OMP_CANCELLATION` at runtime (defaults to "true")
- **New file**: [include/omp.h](include/omp.h) (~200 lines)
  - Complete OpenMP API declarations following OpenMP 5.0 specification
  - Type definitions: `omp_lock_t`, `omp_nest_lock_t`, `omp_sched_t`, etc.
  - Function declarations for all standard OpenMP runtime functions
  - Replaces need for manual `extern "C"` declarations in user code
- **New file**: [src/omp_runtime.cpp](src/omp_runtime.cpp) (~400 lines)
  - Implements all OpenMP runtime API functions
  - Lock types use internal `ncnn::Mutex` with proper nesting support
  - Timing functions use `emscripten_get_now()` for microsecond precision
  - Hierarchy functions support nested parallel regions (currently 1 level)
  - Provides informative stubs for features not yet implemented
- **Example enhancement**: [example/src/data_sharing.cpp](example/src/data_sharing.cpp)
  - Comprehensive demonstration of all data-sharing clause behaviors
  - Shows memory addresses to illustrate private vs shared variables
  - Documents which clauses are compiler-handled vs runtime-handled
  - Clarifies common misconceptions about OpenMP variable scoping

## [1.4.0] - 2025-01-19

### Added
- Support for OpenMP atomic operations (`#pragma omp atomic`):
  - **Arithmetic operations**: `add`, `sub`, `mul`, `div` (integers and floating-point)
  - **Bitwise operations**: `and`, `or`, `xor` (integer types only)
  - **Comparison operations**: `min`, `max` (integers and floating-point)
  - **Memory operations**: `read`, `write` (atomic load/store)
  - **Type support**: 8/16/32/64-bit integers (signed/unsigned), `float`, `double`
  - Example: [example/src/atomic.cpp](example/src/atomic.cpp)
- Support for `nowait` clause (compiler-handled, demonstration example only):
  - Allows skipping implicit barriers at the end of worksharing constructs
  - Works with `#pragma omp for`, `#pragma omp single`, etc.
  - Example: [example/src/nowait.cpp](example/src/nowait.cpp)

### Technical Details
- **New file**: [src/kmp_atomic.cpp](src/kmp_atomic.cpp) (~420 lines)
  - Implements LLVM libomp atomic ABI functions for various operations and types
  - **Integer operations**: Use native atomic `fetch_*` hardware instructions
  - **Floating-point operations**: Use compare-and-swap (CAS) loops for non-native operations
  - **Read/Write operations**: Use atomic `load`/`store` with sequential consistency
  - Covers 32 functions: 8 operations × 4 types (int32/uint32/int64/uint64/float/double)
  - Note: `atomic capture` is not yet implemented
- **`nowait` clause**: Compiler-level feature requiring no runtime implementation
  - Clang omits `__kmpc_barrier()` calls when `nowait` is present in LLVM IR
  - SimpleOMP inherently supports this through existing barrier infrastructure
  - Demonstration example shows performance benefits of skipping implicit barriers

## [1.3.0] - 2025-01-18

### Added
- Support for OpenMP `schedule` clause with dynamic and guided scheduling strategies:
  - **`schedule(dynamic [, chunk_size])`** - Dynamic work distribution
    - Lock-free atomic chunk allocation using compare-and-swap operations
    - Ideal for workloads with irregular iteration costs
    - Configurable chunk size (defaults to 1)
  - **`schedule(guided [, min_chunk])`** - Guided self-scheduling
    - Exponentially decreasing chunk sizes for better load balancing
    - Starts with `remaining_iterations / (2 * num_threads)`, decreases to `min_chunk`
    - Balances overhead reduction with load distribution
  - **`schedule(runtime)`** - Runtime schedule selection
    - Reads scheduling strategy from `OMP_SCHEDULE` environment variable
    - Supports formats: `"static,N"`, `"dynamic,N"`, `"guided,N"`
    - Allows dynamic scheduling decisions without recompilation
  - **`schedule(static, chunk_size)`** - Enhanced static chunked scheduling
    - Round-robin chunk distribution across threads
    - Proper support for arbitrary loop strides (not just `i++`)
- New comprehensive example: [example/src/schedule.cpp](example/src/schedule.cpp)
  - Demonstrates all scheduling strategies with visual iteration distribution
  - Includes performance comparison between static, dynamic, and guided schedules

### Technical Details
- **New file**: [src/kmp_dispatch.cpp](src/kmp_dispatch.cpp) (~900 lines)
  - Implements LLVM libomp dispatch ABI: `__kmpc_dispatch_init_*`, `__kmpc_dispatch_next_*`, `__kmpc_dispatch_deinit`
  - Supports 4 integer types: `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
  - Uses deferred deletion pattern to prevent use-after-free in multi-threaded environments
  - Generation counter mechanism to detect loop reinitialization and prevent race conditions
  - Thread-local storage for tracking expected generation per thread
  - Lock-free algorithms for dynamic chunk allocation with atomic operations
  - Guided scheduling uses exponential decay formula: `chunk = max((upper - current) / (2 * num_threads), min_chunk)`
- **Enhanced**: `__kmpc_for_static_init_4` in [src/simpleomp.cpp](src/simpleomp.cpp)
  - Added support for `schedule(static, chunk)` with proper round-robin distribution
  - Correctly handles arbitrary loop increments/decrements (stride parameter)
  - Extracts base schedule type to ignore modifier flags

## [1.2.0] - 2025-01-17

### Added
- Support for OpenMP synchronization constructs:
  - **`#pragma omp barrier`** - Thread barrier synchronization
    - Implemented `__kmpc_barrier()` function using generation counter pattern
    - Ensures all threads reach the synchronization point before continuing
    - Example: [example/src/barrier.cpp](example/src/barrier.cpp)
  - **`#pragma omp critical`** - Critical sections for mutual exclusion
    - Implemented `__kmpc_critical()` and `__kmpc_end_critical()` functions
    - Supports named critical sections using mutex map
    - Example: [example/src/critical.cpp](example/src/critical.cpp)
  - **`#pragma omp master`** - Master thread-only execution regions
    - Implemented `__kmpc_master()` and `__kmpc_end_master()` functions
    - Simple thread ID check for master thread (thread 0)
    - Example: [example/src/master.cpp](example/src/master.cpp)
  - **`#pragma omp single`** - Single thread execution regions
    - Implemented `__kmpc_single()` and `__kmpc_end_single()` functions
    - Uses map to track first arriving thread per location
    - Example: [example/src/single.cpp](example/src/single.cpp)

### Technical Details
- **Barrier implementation**: Uses mutex, condition variable, and generation counter to handle multiple barrier calls correctly
- **Critical sections**: Each named critical section has its own mutex stored in a thread-safe map
- **Master construct**: Lightweight implementation with simple thread number comparison
- **Single construct**: Tracks execution state per source location to ensure only one thread executes the region
- All synchronization primitives properly integrate with the existing thread pool architecture


## [1.1.0] - 2025-01-17

### Added
- Support for OpenMP `if` clause to conditionally enable/disable parallelization
  - Implemented `__kmpc_serialized_parallel()` function for serial execution path
  - Implemented `__kmpc_end_serialized_parallel()` function for cleanup
  - Example usage: `#pragma omp parallel for if(n > 1000) num_threads(8)`
- New example demonstrating `if` clause usage ([example/src/if.cpp](example/src/if.cpp))
- Example index page ([example/index.html](example/index.html)) for easy navigation between examples

### Changed
- Exported `tls_num_threads` and `tls_thread_num` from `simpleomp.cpp` for use in other compilation units
- Updated coding standards in CLAUDE.md: all code and comments must be written in English

### Technical Details
- The `if` clause allows runtime decisions about parallelization based on conditions
- When the condition evaluates to false, the code executes serially without thread creation overhead
- When the condition evaluates to true, normal parallel execution occurs

## [1.0.0] - 2025-01-14

### Added
- Initial release of SimpleOMP
- Basic OpenMP runtime for Emscripten/WebAssembly
- Support for `#pragma omp parallel for num_threads(N)`
- Static loop scheduling with `__kmpc_for_static_init_*` functions
- Thread pool management using Web Workers
- Example demonstrating parallel for loops
- Comprehensive build system with Makefile
- CI/CD pipeline for automated releases

### Technical Details
- Based on LLVM libomp ABI for Clang compatibility
- Thread-local storage for thread management
- Ring buffer task queue for efficient work distribution
- Derived from Tencent NCNN threading implementation

[1.6.4]: https://github.com/MuTsunTsai/simpleomp/compare/v1.6.3...v1.6.4
[1.6.3]: https://github.com/MuTsunTsai/simpleomp/compare/v1.6.2...v1.6.3
[1.6.2]: https://github.com/MuTsunTsai/simpleomp/compare/v1.6.1...v1.6.2
[1.6.1]: https://github.com/MuTsunTsai/simpleomp/compare/v1.6.0...v1.6.1
[1.6.0]: https://github.com/MuTsunTsai/simpleomp/compare/v1.5.1...v1.6.0
[1.5.1]: https://github.com/MuTsunTsai/simpleomp/compare/v1.5.0...v1.5.1
[1.5.0]: https://github.com/MuTsunTsai/simpleomp/compare/v1.4.0...v1.5.0
[1.4.0]: https://github.com/MuTsunTsai/simpleomp/compare/v1.3.0...v1.4.0
[1.3.0]: https://github.com/MuTsunTsai/simpleomp/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/MuTsunTsai/simpleomp/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/MuTsunTsai/simpleomp/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/MuTsunTsai/simpleomp/releases/tag/v1.0.0
