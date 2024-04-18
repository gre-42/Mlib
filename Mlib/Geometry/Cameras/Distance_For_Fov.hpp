#pragma once
#include <cmath>

namespace Mlib {

template <class T>
T distance_for_fov(const T& fov, const T& radius) {
    auto m = -std::tan(fov / 2);
    return -std::sqrt(m * m + 1) * radius / m;
}

}
