#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scaled_Integer.hpp>
#include <concepts>
#include <eve/std.hpp>

namespace Mlib {

template <std::intmax_t numerator, std::intmax_t denominator>
class PaddedFixedArray3Int32: public FixedArray<ScaledInteger<int32_t, numerator, denominator>, 3> {
public:
    using FixedArray<ScaledInteger<int32_t, numerator, denominator>, 3>::FixedArray;
    PaddedFixedArray3Int32(const FixedArray<ScaledInteger<int32_t, numerator, denominator>, 3>& other)
        : FixedArray<ScaledInteger<int32_t, numerator, denominator>, 3>{ other }
    {}
    int32_t padding = 0;
};

template <std::intmax_t numerator, std::intmax_t denominator>
class PaddedFixedArray3Int16: public FixedArray<ScaledInteger<int16_t, numerator, denominator>, 3> {
public:
    using FixedArray<ScaledInteger<int16_t, numerator, denominator>, 3>::FixedArray;
    PaddedFixedArray3Int16(const FixedArray<ScaledInteger<int16_t, numerator, denominator>, 3>& other)
        : FixedArray<ScaledInteger<int16_t, numerator, denominator>, 3>{ other }
    {}
    int16_t padding = 0;
};

template <std::floating_point TData, size_t tndim>
static FixedArray<TData, tndim> get_padded_fixed_array(const FixedArray<TData, tndim>&) {
    verbose_abort("xx");
}

template <std::intmax_t numerator, std::intmax_t denominator, size_t tndim>
static PaddedFixedArray3Int32<numerator, denominator> get_padded_fixed_array(
    const FixedArray<ScaledInteger<int32_t, numerator, denominator>, tndim>&)
{
    verbose_abort("xx");
}

template <std::intmax_t numerator, std::intmax_t denominator, size_t tndim>
static PaddedFixedArray3Int16<numerator, denominator> get_padded_fixed_array(
    const FixedArray<ScaledInteger<int16_t, numerator, denominator>, tndim>&)
{
    verbose_abort("xx");
}

template <class TData, size_t tndim>
struct padded_fixed_array 
{
    static const FixedArray<TData, tndim>& f();
    using type = decltype(get_padded_fixed_array(f()));
};

template <class TData, size_t tndim>
using padded_fixed_array_t = padded_fixed_array<TData, tndim>::type;

template <std::intmax_t numerator, std::intmax_t denominator>
bool all_le(
    const PaddedFixedArray3Int32<numerator, denominator>& a,
    const PaddedFixedArray3Int32<numerator, denominator>& b)
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

template <std::intmax_t numerator, std::intmax_t denominator>
bool all_ge(
    const PaddedFixedArray3Int32<numerator, denominator>& a,
    const PaddedFixedArray3Int32<numerator, denominator>& b)
{
    return all_le(b, a);
}

template <std::intmax_t numerator, std::intmax_t denominator>
bool all_le(
    const PaddedFixedArray3Int16<numerator, denominator>& a,
    const PaddedFixedArray3Int16<numerator, denominator>& b)
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

template <std::intmax_t numerator, std::intmax_t denominator>
bool all_ge(
    const PaddedFixedArray3Int16<numerator, denominator>& a,
    const PaddedFixedArray3Int16<numerator, denominator>& b)
{
    return all_le(b, a);
}

template <std::floating_point TFloat, size_t tndim>
bool all_le(
    const FixedArray<TFloat, tndim>& a,
    const FixedArray<TFloat, tndim>& b)
{
    for (size_t i = 0; i < tndim; ++i) {
        if (a(i) > b(i)) {
            return false;
        }
    }
    return true;
}

template <std::floating_point TFloat, size_t tndim>
bool all_ge(
    const FixedArray<TFloat, tndim>& a,
    const FixedArray<TFloat, tndim>& b)
{
    return all_le(b, a);
}

}
