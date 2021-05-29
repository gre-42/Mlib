#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <class TData>
inline TData det2x2(const FixedArray<TData, 2, 2>& a)
{
    return a(0, 0) * a(1, 1) - a(0, 1) * a(1, 0);
}

}
