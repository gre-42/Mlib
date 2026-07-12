#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
void lerp_array(
    const TData& a,
    const TData& b,
    Array<TData>& alpha,
    const TData& low = 0, const TData& high = 1)
{
    alpha -= low;
    alpha *= (b - a) / (high - low);
    alpha += a;
}

}
