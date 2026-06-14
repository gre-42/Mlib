#pragma once
#include <cmath>
#include <stdexcept>

namespace Mlib {

template <class T>
inline T angle_pi_pi(T a) {
    constexpr const auto pi = T(M_PI);
    constexpr const auto pi11 = T(1.1 * M_PI);
    constexpr const auto pi2 = T(2. * M_PI);
    if (a > pi) {
        a -= pi2;
    }
    if (a < -pi) {
        a += pi2;
    }
    if (a > pi11) {
        throw std::runtime_error("Angle above 1.1*pi");
    }
    if (a < -pi11) {
        throw std::runtime_error("Angle below -1.1*pi");
    }
    return a;
}

}
