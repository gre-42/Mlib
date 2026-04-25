// Copyright 2025 Mu-Tsun Tsai
// Licensed under the MIT License

// Atomic operations support for SimpleOMP
// Implements __kmpc_atomic_* functions (LLVM libomp ABI)

#include <atomic>
#include <cstdint>

// Forward declarations for OpenMP runtime types
struct ident_t;

//==============================================================================
// Template-based atomic operation implementations
//==============================================================================

// Generic atomic add operation for all types
template<typename T>
inline void atomic_add_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    atomic_lhs->fetch_add(rhs, std::memory_order_seq_cst);
}

// Specialization for floating point types using CAS loop
// Note: fetch_add is not available for float/double in all implementations
template<>
inline void atomic_add_impl<float>(float* lhs, float rhs) {
    std::atomic<float>* atomic_lhs = reinterpret_cast<std::atomic<float>*>(lhs);
    float expected = atomic_lhs->load(std::memory_order_relaxed);
    while (!atomic_lhs->compare_exchange_weak(
        expected,
        expected + rhs,
        std::memory_order_seq_cst,
        std::memory_order_relaxed
    )) {
        // CAS loop: retry until success
    }
}

template<>
inline void atomic_add_impl<double>(double* lhs, double rhs) {
    std::atomic<double>* atomic_lhs = reinterpret_cast<std::atomic<double>*>(lhs);
    double expected = atomic_lhs->load(std::memory_order_relaxed);
    while (!atomic_lhs->compare_exchange_weak(
        expected,
        expected + rhs,
        std::memory_order_seq_cst,
        std::memory_order_relaxed
    )) {
        // CAS loop: retry until success
    }
}

//==============================================================================
// Macro to generate __kmpc_atomic_* API functions
//==============================================================================

// Parameters: loc and gtid are unused in this minimal implementation
// In full libomp, they're used for debugging and thread tracking
#define DEFINE_ATOMIC_ADD(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_add(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; \
    (void)gtid; \
    atomic_add_impl(lhs, rhs); \
}

//==============================================================================
// Integer atomic operations
//==============================================================================

// 8-bit integer (char)
DEFINE_ATOMIC_ADD(fixed1, int8_t)
DEFINE_ATOMIC_ADD(fixed1u, uint8_t)

// 16-bit integer (short)
DEFINE_ATOMIC_ADD(fixed2, int16_t)
DEFINE_ATOMIC_ADD(fixed2u, uint16_t)

// 32-bit integer (int)
DEFINE_ATOMIC_ADD(fixed4, int32_t)
DEFINE_ATOMIC_ADD(fixed4u, uint32_t)

// 64-bit integer (long long)
DEFINE_ATOMIC_ADD(fixed8, int64_t)
DEFINE_ATOMIC_ADD(fixed8u, uint64_t)

//==============================================================================
// Floating-point atomic operations
//==============================================================================

// 32-bit float
DEFINE_ATOMIC_ADD(float4, float)

// 64-bit double
DEFINE_ATOMIC_ADD(float8, double)

//==============================================================================
// Subtraction operations
//==============================================================================

template<typename T>
inline void atomic_sub_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    atomic_lhs->fetch_sub(rhs, std::memory_order_seq_cst);
}

template<>
inline void atomic_sub_impl<float>(float* lhs, float rhs) {
    std::atomic<float>* atomic_lhs = reinterpret_cast<std::atomic<float>*>(lhs);
    float expected = atomic_lhs->load(std::memory_order_relaxed);
    while (!atomic_lhs->compare_exchange_weak(
        expected,
        expected - rhs,
        std::memory_order_seq_cst,
        std::memory_order_relaxed
    )) {}
}

template<>
inline void atomic_sub_impl<double>(double* lhs, double rhs) {
    std::atomic<double>* atomic_lhs = reinterpret_cast<std::atomic<double>*>(lhs);
    double expected = atomic_lhs->load(std::memory_order_relaxed);
    while (!atomic_lhs->compare_exchange_weak(
        expected,
        expected - rhs,
        std::memory_order_seq_cst,
        std::memory_order_relaxed
    )) {}
}

#define DEFINE_ATOMIC_SUB(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_sub(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_sub_impl(lhs, rhs); \
}

DEFINE_ATOMIC_SUB(fixed1, int8_t)
DEFINE_ATOMIC_SUB(fixed1u, uint8_t)
DEFINE_ATOMIC_SUB(fixed2, int16_t)
DEFINE_ATOMIC_SUB(fixed2u, uint16_t)
DEFINE_ATOMIC_SUB(fixed4, int32_t)
DEFINE_ATOMIC_SUB(fixed4u, uint32_t)
DEFINE_ATOMIC_SUB(fixed8, int64_t)
DEFINE_ATOMIC_SUB(fixed8u, uint64_t)
DEFINE_ATOMIC_SUB(float4, float)
DEFINE_ATOMIC_SUB(float8, double)

//==============================================================================
// Multiplication operations
//==============================================================================

template<typename T>
inline void atomic_mul_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    T expected = atomic_lhs->load(std::memory_order_relaxed);
    while (!atomic_lhs->compare_exchange_weak(
        expected,
        expected * rhs,
        std::memory_order_seq_cst,
        std::memory_order_relaxed
    )) {}
}

#define DEFINE_ATOMIC_MUL(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_mul(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_mul_impl(lhs, rhs); \
}

DEFINE_ATOMIC_MUL(fixed1, int8_t)
DEFINE_ATOMIC_MUL(fixed1u, uint8_t)
DEFINE_ATOMIC_MUL(fixed2, int16_t)
DEFINE_ATOMIC_MUL(fixed2u, uint16_t)
DEFINE_ATOMIC_MUL(fixed4, int32_t)
DEFINE_ATOMIC_MUL(fixed4u, uint32_t)
DEFINE_ATOMIC_MUL(fixed8, int64_t)
DEFINE_ATOMIC_MUL(fixed8u, uint64_t)
DEFINE_ATOMIC_MUL(float4, float)
DEFINE_ATOMIC_MUL(float8, double)

//==============================================================================
// Division operations
//==============================================================================

template<typename T>
inline void atomic_div_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    T expected = atomic_lhs->load(std::memory_order_relaxed);
    while (!atomic_lhs->compare_exchange_weak(
        expected,
        expected / rhs,
        std::memory_order_seq_cst,
        std::memory_order_relaxed
    )) {}
}

#define DEFINE_ATOMIC_DIV(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_div(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_div_impl(lhs, rhs); \
}

DEFINE_ATOMIC_DIV(fixed1, int8_t)
DEFINE_ATOMIC_DIV(fixed1u, uint8_t)
DEFINE_ATOMIC_DIV(fixed2, int16_t)
DEFINE_ATOMIC_DIV(fixed2u, uint16_t)
DEFINE_ATOMIC_DIV(fixed4, int32_t)
DEFINE_ATOMIC_DIV(fixed4u, uint32_t)
DEFINE_ATOMIC_DIV(fixed8, int64_t)
DEFINE_ATOMIC_DIV(fixed8u, uint64_t)
DEFINE_ATOMIC_DIV(float4, float)
DEFINE_ATOMIC_DIV(float8, double)

//==============================================================================
// Bitwise AND operations (integer only)
//==============================================================================

template<typename T>
inline void atomic_and_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    atomic_lhs->fetch_and(rhs, std::memory_order_seq_cst);
}

#define DEFINE_ATOMIC_AND(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_andb(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_and_impl(lhs, rhs); \
}

DEFINE_ATOMIC_AND(fixed1, int8_t)
DEFINE_ATOMIC_AND(fixed1u, uint8_t)
DEFINE_ATOMIC_AND(fixed2, int16_t)
DEFINE_ATOMIC_AND(fixed2u, uint16_t)
DEFINE_ATOMIC_AND(fixed4, int32_t)
DEFINE_ATOMIC_AND(fixed4u, uint32_t)
DEFINE_ATOMIC_AND(fixed8, int64_t)
DEFINE_ATOMIC_AND(fixed8u, uint64_t)

//==============================================================================
// Bitwise OR operations (integer only)
//==============================================================================

template<typename T>
inline void atomic_or_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    atomic_lhs->fetch_or(rhs, std::memory_order_seq_cst);
}

#define DEFINE_ATOMIC_OR(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_orb(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_or_impl(lhs, rhs); \
}

DEFINE_ATOMIC_OR(fixed1, int8_t)
DEFINE_ATOMIC_OR(fixed1u, uint8_t)
DEFINE_ATOMIC_OR(fixed2, int16_t)
DEFINE_ATOMIC_OR(fixed2u, uint16_t)
DEFINE_ATOMIC_OR(fixed4, int32_t)
DEFINE_ATOMIC_OR(fixed4u, uint32_t)
DEFINE_ATOMIC_OR(fixed8, int64_t)
DEFINE_ATOMIC_OR(fixed8u, uint64_t)

//==============================================================================
// Bitwise XOR operations (integer only)
//==============================================================================

template<typename T>
inline void atomic_xor_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    atomic_lhs->fetch_xor(rhs, std::memory_order_seq_cst);
}

#define DEFINE_ATOMIC_XOR(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_xor(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_xor_impl(lhs, rhs); \
}

DEFINE_ATOMIC_XOR(fixed1, int8_t)
DEFINE_ATOMIC_XOR(fixed1u, uint8_t)
DEFINE_ATOMIC_XOR(fixed2, int16_t)
DEFINE_ATOMIC_XOR(fixed2u, uint16_t)
DEFINE_ATOMIC_XOR(fixed4, int32_t)
DEFINE_ATOMIC_XOR(fixed4u, uint32_t)
DEFINE_ATOMIC_XOR(fixed8, int64_t)
DEFINE_ATOMIC_XOR(fixed8u, uint64_t)

//==============================================================================
// Min operations
//==============================================================================

template<typename T>
inline void atomic_min_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    T expected = atomic_lhs->load(std::memory_order_relaxed);
    while (rhs < expected) {
        if (atomic_lhs->compare_exchange_weak(
            expected,
            rhs,
            std::memory_order_seq_cst,
            std::memory_order_relaxed
        )) {
            break;
        }
    }
}

#define DEFINE_ATOMIC_MIN(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_min(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_min_impl(lhs, rhs); \
}

DEFINE_ATOMIC_MIN(fixed1, int8_t)
DEFINE_ATOMIC_MIN(fixed1u, uint8_t)
DEFINE_ATOMIC_MIN(fixed2, int16_t)
DEFINE_ATOMIC_MIN(fixed2u, uint16_t)
DEFINE_ATOMIC_MIN(fixed4, int32_t)
DEFINE_ATOMIC_MIN(fixed4u, uint32_t)
DEFINE_ATOMIC_MIN(fixed8, int64_t)
DEFINE_ATOMIC_MIN(fixed8u, uint64_t)
DEFINE_ATOMIC_MIN(float4, float)
DEFINE_ATOMIC_MIN(float8, double)

//==============================================================================
// Max operations
//==============================================================================

template<typename T>
inline void atomic_max_impl(T* lhs, T rhs) {
    std::atomic<T>* atomic_lhs = reinterpret_cast<std::atomic<T>*>(lhs);
    T expected = atomic_lhs->load(std::memory_order_relaxed);
    while (rhs > expected) {
        if (atomic_lhs->compare_exchange_weak(
            expected,
            rhs,
            std::memory_order_seq_cst,
            std::memory_order_relaxed
        )) {
            break;
        }
    }
}

#define DEFINE_ATOMIC_MAX(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_max(ident_t* loc, int gtid, type* lhs, type rhs) { \
    (void)loc; (void)gtid; \
    atomic_max_impl(lhs, rhs); \
}

DEFINE_ATOMIC_MAX(fixed1, int8_t)
DEFINE_ATOMIC_MAX(fixed1u, uint8_t)
DEFINE_ATOMIC_MAX(fixed2, int16_t)
DEFINE_ATOMIC_MAX(fixed2u, uint16_t)
DEFINE_ATOMIC_MAX(fixed4, int32_t)
DEFINE_ATOMIC_MAX(fixed4u, uint32_t)
DEFINE_ATOMIC_MAX(fixed8, int64_t)
DEFINE_ATOMIC_MAX(fixed8u, uint64_t)
DEFINE_ATOMIC_MAX(float4, float)
DEFINE_ATOMIC_MAX(float8, double)

//==============================================================================
// Atomic read operations
//==============================================================================

template<typename T>
inline T atomic_read_impl(T* ptr) {
    std::atomic<T>* atomic_ptr = reinterpret_cast<std::atomic<T>*>(ptr);
    return atomic_ptr->load(std::memory_order_seq_cst);
}

#define DEFINE_ATOMIC_READ(suffix, type) \
extern "C" type __kmpc_atomic_##suffix##_rd(ident_t* loc, int gtid, type* ptr) { \
    (void)loc; (void)gtid; \
    return atomic_read_impl(ptr); \
}

DEFINE_ATOMIC_READ(fixed1, int8_t)
DEFINE_ATOMIC_READ(fixed1u, uint8_t)
DEFINE_ATOMIC_READ(fixed2, int16_t)
DEFINE_ATOMIC_READ(fixed2u, uint16_t)
DEFINE_ATOMIC_READ(fixed4, int32_t)
DEFINE_ATOMIC_READ(fixed4u, uint32_t)
DEFINE_ATOMIC_READ(fixed8, int64_t)
DEFINE_ATOMIC_READ(fixed8u, uint64_t)
DEFINE_ATOMIC_READ(float4, float)
DEFINE_ATOMIC_READ(float8, double)

//==============================================================================
// Atomic write operations
//==============================================================================

template<typename T>
inline void atomic_write_impl(T* ptr, T value) {
    std::atomic<T>* atomic_ptr = reinterpret_cast<std::atomic<T>*>(ptr);
    atomic_ptr->store(value, std::memory_order_seq_cst);
}

#define DEFINE_ATOMIC_WRITE(suffix, type) \
extern "C" void __kmpc_atomic_##suffix##_wr(ident_t* loc, int gtid, type* ptr, type value) { \
    (void)loc; (void)gtid; \
    atomic_write_impl(ptr, value); \
}

DEFINE_ATOMIC_WRITE(fixed1, int8_t)
DEFINE_ATOMIC_WRITE(fixed1u, uint8_t)
DEFINE_ATOMIC_WRITE(fixed2, int16_t)
DEFINE_ATOMIC_WRITE(fixed2u, uint16_t)
DEFINE_ATOMIC_WRITE(fixed4, int32_t)
DEFINE_ATOMIC_WRITE(fixed4u, uint32_t)
DEFINE_ATOMIC_WRITE(fixed8, int64_t)
DEFINE_ATOMIC_WRITE(fixed8u, uint64_t)
DEFINE_ATOMIC_WRITE(float4, float)
DEFINE_ATOMIC_WRITE(float8, double)
