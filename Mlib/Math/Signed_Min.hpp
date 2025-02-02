#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class T>
T signed_min(const T& v, const T& max_length) {
    return sign(v) * std::min(std::abs(v), max_length);
}

}
