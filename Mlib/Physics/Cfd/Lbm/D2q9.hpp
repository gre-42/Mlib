#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <array>

namespace Mlib {

static const std::array<FixedArray<int, 2>, 9> discrete_velocity_directions_d2q9 = {
    FixedArray<int, 2>{1, -1}, FixedArray<int, 2>{1, 0}, FixedArray<int, 2>{1, 1},
    FixedArray<int, 2>{0, -1}, FixedArray<int, 2>{0, 0}, FixedArray<int, 2>{0, 1},
    FixedArray<int, 2>{-1, -1}, FixedArray<int, 2>{-1, 0}, FixedArray<int, 2>{-1, 1}};
    
template <class T>
struct LbmModelD2Q9 {
    using type = T;

    static const size_t ndirections = 9;

    constexpr static const T weights[ndirections] = {
        (T)1 / 36, (T)1 / 9, (T)1 / 36,
        (T)1 / 9, (T)4 / 9, (T)1 / 9,
        (T)1 / 36, (T)1 / 9, (T)1 / 36};

    constexpr static const std::array<FixedArray<int, 2>, ndirections>& discrete_velocity_directions =
        discrete_velocity_directions_d2q9;
};

}
