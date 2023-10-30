#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
FixedArray<TData, 2, 2> fixed_rotation_2d(TData angle) {
    return FixedArray<TData, 2, 2>::init(
        std::cos(angle), -std::sin(angle),
        std::sin(angle), std::cos(angle));
}

}
