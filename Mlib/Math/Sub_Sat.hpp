#pragma once
#include <concepts>

namespace Mlib {

template <std::unsigned_integral T>
T sub_sat(T a, T b) {
    return (b >= a) ? 0 : a - b;
}

}
