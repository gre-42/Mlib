#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData, class TAlpha>
auto lerp_array(const TData& a, const TData& b, const Array<TAlpha>& alpha) {
    return a + alpha * (b - a);
}

}
