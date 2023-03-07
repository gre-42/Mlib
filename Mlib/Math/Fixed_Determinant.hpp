#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <class TData>
inline TData det2x2(const FixedArray<TData, 2, 2>& a)
{
    return a(0u, 0u) * a(1u, 1u) - a(0u, 1u) * a(1u, 0u);
}

template <class TData>
inline TData det3x3(const FixedArray<TData, 3, 3>& a)
{
    return
        a(0u, 0u) * (a(1u, 1u) * a(2u, 2u) - a(1u, 2u) * a(2u, 1u)) -
        a(0u, 1u) * (a(1u, 0u) * a(2u, 2u) - a(1u, 2u) * a(2u, 0u)) +
        a(0u, 2u) * (a(1u, 0u) * a(2u, 1u) - a(1u, 1u) * a(2u, 0u));
}

}
