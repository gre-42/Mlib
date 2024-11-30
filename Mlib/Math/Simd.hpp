#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scaled_Integer.hpp>
#include <concepts>
#include <eve/std.hpp>

namespace Mlib {

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
auto all_le(
    const FixedArray<ScaledInteger<TInt, numerator, denominator>, 3>& a,
    const FixedArray<ScaledInteger<TInt, numerator, denominator>, 3>& b)
{
    eve::experimental::fixed_size_simd<TInt, 4> ea;
    ea.set(0, a(0).count);
    ea.set(1, a(1).count);
    ea.set(2, a(2).count);
    ea.set(3, 0);

    eve::experimental::fixed_size_simd<TInt, 4> eb;
    eb.set(0, b(0).count);
    eb.set(1, b(1).count);
    eb.set(2, b(2).count);
    eb.set(3, 0);
    return eve::all(ea <= eb);
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
auto all_ge(
    const FixedArray<ScaledInteger<TInt, numerator, denominator>, 3>& a,
    const FixedArray<ScaledInteger<TInt, numerator, denominator>, 3>& b)
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
