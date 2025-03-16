#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class T>
T curvature(const FixedArray<T, 3, 2>& points) {
    // From: https://en.wikipedia.org/wiki/Curvature#In_terms_of_a_general_parametrization
    auto d1 = 0.5 * (points[2] - points[0]);
    auto d2 = points[2] - 2. * points[1] + points[0];
    auto x1 = d1(0);
    auto x2 = d2(0);
    auto y1 = d1(1);
    auto y2 = d2(1);
    return (x1 * y2 - y1 * x2) / std::pow(squared(x1) + squared(y1), 3. / 2.);
}

// template <class T>
// T curvature_x(const FixedArray<FixedArray<T, 2>, 3>& points) {
//     auto v = points(2) - points(0);
//     FixedArray<T, 2> n{v(1), -v(0)};
//     T len2 = sum(squared(n));
//     T len = std::sqrt(len2);
//     n /= len;
//     auto diff = dot0d(n, 0.5 * (points(0) + points(2)) - points(1));
//     return -8. * diff / len2;
// }

}
