#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData, size_t tndim>
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
inline bool all_ge(
    const FixedArray<TData, tndim>& a,
    const FixedArray<TData, tndim>& b)
{
    return all_le(b, a);
}

}
