# SimpleOMP

[![GitHub Release](https://img.shields.io/github/v/release/MuTsunTsai/simpleomp)](https://github.com/MuTsunTsai/simpleomp/releases)
[![Build Status](https://img.shields.io/github/actions/workflow/status/MuTsunTsai/simpleomp/deploy.yml?branch=main)](https://github.com/MuTsunTsai/simpleomp/actions)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Live Demo](https://img.shields.io/badge/demo-live-success)](https://MuTsunTsai.github.io/simpleomp/)

A lightweight OpenMP implementation for Emscripten, enabling basic parallel programming capabilities in WebAssembly applications.

## Overview

SimpleOMP provides a minimal OpenMP runtime for Emscripten-compiled projects. This implementation is based on the solution discussed in [emscripten-core/emscripten#13892](https://github.com/emscripten-core/emscripten/issues/13892#issuecomment-2599113825).

### Supported Features

#### Parallel Execution

| Directive/Clause | Status | Description |
|------------------|--------|-------------|
| `#pragma omp parallel for` | ✅ | Parallel for loop |
| `num_threads(N)` | ✅ | Specify thread count |
| `if(condition)` | ✅ | Conditional parallelization |
| `schedule(static[, chunk])` | ✅ | Static loop scheduling (block or round-robin) |
| `schedule(dynamic[, chunk])` | ✅ | Dynamic work distribution |
| `schedule(guided[, chunk])` | ✅ | Guided scheduling with decreasing chunk sizes |
| `schedule(runtime)` | ✅ | Runtime-determined scheduling (via `OMP_SCHEDULE`) |

#### Synchronization

| Directive/Clause | Status | Description |
|------------------|--------|-------------|
| `#pragma omp barrier` | ✅ | Thread barrier synchronization |
| `#pragma omp critical [(name)]` | ✅ | Critical section (mutual exclusion) |
| `#pragma omp master` | ✅ | Master thread-only execution |
| `#pragma omp single` | ✅ | Single thread execution |
| `#pragma omp atomic` | ⚠️ | Atomic operations (partial: add/sub/mul/div/and/or/xor/min/max/read/write; missing: capture) |
| `#pragma omp cancel` | ✅ | Request cancellation of parallel regions or loops (controlled by `OMP_CANCELLATION`) |
| `#pragma omp cancellation point` | ✅ | Check for cancellation requests |
| `#pragma omp ordered` | ❌ | Ordered execution within parallel loops |

#### Work Sharing

| Directive/Clause | Status | Description |
|------------------|--------|-------------|
| `#pragma omp sections` | ❌ | Separate code sections |
| `#pragma omp task` | ❌ | Task-based parallelism |

#### Advanced Clauses

| Directive/Clause | Status | Description |
|------------------|--------|-------------|
| `nowait` | ✅ | Skip implicit barrier at end of worksharing constructs (compiler-handled, no runtime support needed) |
| `copyprivate(var)` | ❌ | Broadcast private variable from `single` to all threads |

#### Data Environment

| Directive/Clause | Status | Description |
|------------------|--------|-------------|
| `reduction(op:var)` | ✅ | Reduction operations (supports +, *, -, &, \|, ^, &&, \|\|, min, max) |
| `private(var)` | ✅ | Thread-private variables (compiler-handled, no runtime support needed) |
| `shared(var)` | ✅ | Shared variables (compiler-handled, no runtime support needed) |
| `firstprivate(var)` | ✅ | Initialize private from shared (compiler-handled, no runtime support needed) |
| `lastprivate(var)` | ✅ | Update shared from last iteration (compiler-handled, no runtime support needed) |
| `default(shared\|none)` | ✅ | Default data-sharing attribute (compiler-only, no runtime support needed) |
| `threadprivate` | ❌ | Thread-private global variables (requires deep compiler integration) |
| `copyin(var)` | ❌ | Initialize threadprivate variables (depends on threadprivate) |

#### Not Applicable to WebAssembly

| Directive/Clause | Status | Description |
|------------------|--------|-------------|
| `#pragma omp flush` | 🚫 | Memory fence (WebAssembly atomics already provide memory ordering) |
| `#pragma omp target` | 🚫 | Offload to accelerator devices (not applicable to Wasm environment) |

### Known Limitations

#### Nested Parallelism

**Nested parallel regions are NOT supported.** SimpleOMP uses a single global thread pool architecture and does not support hierarchical team structures. Attempting to use nested `#pragma omp parallel` will result in undefined behavior (potential deadlock or serialization of inner regions).

```cpp
// ❌ NOT SUPPORTED
#pragma omp parallel num_threads(4)
{
    // Outer parallel region: 4 threads
    #pragma omp parallel num_threads(2)
    {
        // Inner parallel region: This will cause issues!
    }
}
```

**Workaround:** Restructure your algorithm to avoid nested parallelism, or flatten the parallel structure into a single level.

The following OpenMP Runtime API functions related to nested parallelism are provided as stubs that always return fixed values:
- `omp_set_nested()` / `omp_get_nested()` - Always reports nested parallelism as disabled
- `omp_set_max_active_levels()` / `omp_get_max_active_levels()` - Always reports 1 active level
- `omp_get_level()` - Always returns 0 (not inside a parallel region) or 1 (inside a parallel region)
- `omp_get_active_level()` - Same as `omp_get_level()`
- `omp_get_ancestor_thread_num(level)` - Only works for level 0 or 1
- `omp_get_team_size(level)` - Only works for level 0 or 1

## Usage

### Prerequisites

- Emscripten toolchain
- Make

### Installation

1. Download `libsimpleomp.a` from the [releases](../../releases) page
2. (Optional) Download `omp.h` if you need to use OpenMP Runtime API functions (e.g., `omp_get_thread_num()`, `omp_set_num_threads()`, locks, timing functions)
3. Link the library when building your project
4. Add the following compilation flags: `-fopenmp -pthread`

### Example

```bash
# Basic usage (pragma directives only)
emcc your_code.c -fopenmp -pthread \
  -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency \
  libsimpleomp.a -o output.js

# With OpenMP Runtime API (using omp.h)
emcc your_code.c -I/path/to/include -fopenmp -pthread \
  -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency \
  libsimpleomp.a -o output.js
```

**⚠️ IMPORTANT:** You **MUST** include the `-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency` flag when linking. SimpleOMP creates a worker thread pool sized to match the number of logical CPU cores. Without this flag, Emscripten's default pthread pool size will be insufficient, causing runtime errors.

For a complete working example, see the [example](example/) directory.

## Building from Source

To build the library from source:

```bash
# Build the library
make

# Output will be generated at: dist/libsimpleomp.a
```

## Running the Examples

The [example](example/) directory contains sample projects demonstrating SimpleOMP usage:
- **for.cpp** - Basic parallel for loop with performance comparison
- **if.cpp** - Conditional parallelization using the `if` clause
- **schedule.cpp** - Loop scheduling strategies (static/dynamic/guided) with validation
- **master.cpp** - Master thread construct demonstration
- **critical.cpp** - Critical section for mutual exclusion
- **barrier.cpp** - Barrier synchronization example
- **single.cpp** - Single thread execution construct
- **atomic.cpp** - Atomic operations (add/sub/mul/div/and/or/xor/min/max/read/write)
- **nowait.cpp** - Nowait clause demonstration (skipping implicit barriers)
- **locks.cpp** - OpenMP lock API demonstration (simple and nestable locks)
- **data_sharing.cpp** - Data-sharing clauses (private/shared/firstprivate/lastprivate)
- **cancel.cpp** - Cancellation constructs for early termination of parallel regions
- **reduction.cpp** - Reduction operations (sum, product, min/max, logical, bitwise)

```bash
# Build all examples
make with-examples

# Start a local server to test (requires PNPM)
make serve

# Open the URL that appears to see all examples
```

## License

This project is licensed under the MIT License - Copyright (c) 2025 Mu-Tsun Tsai.

### Third-Party Licenses

The following source files are derived from [Tencent NCNN](https://github.com/Tencent/ncnn) and are licensed under the BSD 3-Clause License:

- [src/cpu.h](src/cpu.h)
- [src/platform.h](src/platform.h)
- [src/simpleomp.cpp](src/simpleomp.cpp)

See the respective files for their copyright notices and license terms.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Acknowledgments

This project is based on the implementation discussed in the Emscripten issue tracker. Special thanks to:
- The contributors of [emscripten-core/emscripten#13892](https://github.com/emscripten-core/emscripten/issues/13892)
- The [Tencent NCNN](https://github.com/Tencent/ncnn) project for the threading implementation code