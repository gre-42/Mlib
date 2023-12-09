#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <class TData>
inline TData trace2x2(const FixedArray<TData, 2, 2>& a)
{
    return a(0, 0) + a(1, 1);
}

template <class TData>
inline TData trace3x3(const FixedArray<TData, 3, 3>& a)
{
    return a(0, 0) + a(1, 1) + a(2, 2);
}

}
