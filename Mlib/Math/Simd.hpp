#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <concepts>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <eve/std.hpp>
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

namespace Mlib {

template <class T, size_t tndim>
consteval bool requires_simd_optimization() {
    if (tndim % 2 == 0) {
        return false;
    }
    if (std::is_same_v<T, double>) {
        return false;
    }
    if (std::is_same_v<T, uint64_t>) {
        return false;
    }
    return true;
}

class PaddedFixedArray3Uint32: public FixedArray<uint32_t, 3> {
public:
    using FixedArray<uint32_t, 3>::FixedArray;
    explicit PaddedFixedArray3Uint32(const FixedArray<uint32_t, 3>& other)
        : FixedArray<uint32_t, 3>{ other }
    {}
    uint32_t padding = 0;
};

template <std::intmax_t denominator>
class PaddedFixedArray3FpInt32: public FixedArray<FixedPointNumber<int32_t, denominator>, 3> {
public:
    using FixedArray<FixedPointNumber<int32_t, denominator>, 3>::FixedArray;
    explicit PaddedFixedArray3FpInt32(const FixedArray<FixedPointNumber<int32_t, denominator>, 3>& other)
        : FixedArray<FixedPointNumber<int32_t, denominator>, 3>{ other }
    {}
    int32_t padding = 0;
};

template <std::intmax_t denominator>
class PaddedFixedArray3FpInt16: public FixedArray<FixedPointNumber<int16_t, denominator>, 3> {
public:
    using FixedArray<FixedPointNumber<int16_t, denominator>, 3>::FixedArray;
    explicit PaddedFixedArray3FpInt16(const FixedArray<FixedPointNumber<int16_t, denominator>, 3>& other)
        : FixedArray<FixedPointNumber<int16_t, denominator>, 3>{ other }
    {}
    int16_t padding = 0;
};

class PaddedFixedArray3Float: public FixedArray<float, 3> {
public:
    using FixedArray<float, 3>::FixedArray;
    explicit PaddedFixedArray3Float(const FixedArray<float, 3>& other)
        : FixedArray<float, 3>{ other }
    {}
    float padding = 0;
};

template <class TData, size_t tndim>
struct padded_fixed_array
{
    static_assert(!requires_simd_optimization<TData, tndim>());
    using type = FixedArray<TData, tndim>;
};

template<>
struct padded_fixed_array<uint32_t, 3> {
    using type = PaddedFixedArray3Uint32;
};

template<>
struct padded_fixed_array<FixedPointNumber<int32_t, SCENE_POS_DENOMINATOR>, 3> {
    using type = PaddedFixedArray3FpInt32<SCENE_POS_DENOMINATOR>;
};

template<>
struct padded_fixed_array<FixedPointNumber<int16_t, SCENE_POS_DENOMINATOR>, 3> {
    using type = PaddedFixedArray3FpInt16<SCENE_POS_DENOMINATOR>;
};

template<>
struct padded_fixed_array<float, 3> {
    using type = PaddedFixedArray3Float;
};

template <class TData, size_t tndim>
using padded_fixed_array_t = padded_fixed_array<TData, tndim>::type;

inline bool all_le(
    const PaddedFixedArray3Uint32& a,
    const PaddedFixedArray3Uint32& b)
{
    eve::experimental::fixed_size_simd<uint32_t, 4> ea(
        a(0),
        a(1),
        a(2),
        a.padding);

    eve::experimental::fixed_size_simd<uint32_t, 4> eb(
        b(0),
        b(1),
        b(2),
        b.padding);
    return eve::all(ea <= eb);
}

inline bool all_ge(
    const PaddedFixedArray3Uint32& a,
    const PaddedFixedArray3Uint32& b)
{
    return all_le(b, a);
}

template <std::intmax_t denominator>
inline bool all_le(
    const PaddedFixedArray3FpInt32<denominator>& a,
    const PaddedFixedArray3FpInt32<denominator>& b)
{
    eve::experimental::fixed_size_simd<int32_t, 4> ea(
        a(0).count,
        a(1).count,
        a(2).count,
        a.padding);

    eve::experimental::fixed_size_simd<int32_t, 4> eb(
        b(0).count,
        b(1).count,
        b(2).count,
        b.padding);
    return eve::all(ea <= eb);
}

template <std::intmax_t denominator>
inline bool all_ge(
    const PaddedFixedArray3FpInt32<denominator>& a,
    const PaddedFixedArray3FpInt32<denominator>& b)
{
    return all_le(b, a);
}

template <std::intmax_t denominator>
inline bool all_le(
    const PaddedFixedArray3FpInt16<denominator>& a,
    const PaddedFixedArray3FpInt16<denominator>& b)
{
    eve::experimental::fixed_size_simd<int16_t, 4> ea(
        a(0).count,
        a(1).count,
        a(2).count,
        a.padding);

    eve::experimental::fixed_size_simd<int16_t, 4> eb(
        b(0).count,
        b(1).count,
        b(2).count,
        b.padding);
    return eve::all(ea <= eb);
}

template <std::intmax_t denominator>
inline bool all_ge(
    const PaddedFixedArray3FpInt16<denominator>& a,
    const PaddedFixedArray3FpInt16<denominator>& b)
{
    return all_le(b, a);
}

inline bool all_le(
    const PaddedFixedArray3Float& a,
    const PaddedFixedArray3Float& b)
{
    eve::experimental::fixed_size_simd<float, 4> ea(
        a(0),
        a(1),
        a(2),
        a.padding);

    eve::experimental::fixed_size_simd<float, 4> eb(
        b(0),
        b(1),
        b(2),
        b.padding);
    return eve::all(ea <= eb);
}

inline bool all_ge(
    const PaddedFixedArray3Float& a,
    const PaddedFixedArray3Float& b)
{
    return all_le(b, a);
}

template <class TData, size_t tndim>
    requires (!requires_simd_optimization<TData, tndim>())
inline bool all_le(
    const FixedArray<TData, tndim>& a,
    const FixedArray<TData, tndim>& b)
{
    for (size_t i = 0; i < tndim; ++i) {
        if (a(i) > b(i)) {
            return false;
        }
    }
    return true;
}

template <class TData, size_t tndim>
    requires (!requires_simd_optimization<TData, tndim>())
inline bool all_ge(
    const FixedArray<TData, tndim>& a,
    const FixedArray<TData, tndim>& b)
{
    return all_le(b, a);
}

}
