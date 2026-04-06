#pragma once
#include <cmath>
#include <concepts>
#include <stdexcept>
#include <string>

namespace Mlib {

template <std::integral TDest, std::floating_point TSource>
TDest float_to_integral(TSource source) {
    if (!std::isfinite(source)) {
        throw std::runtime_error("float_to_integral: Floating-point number is not finite");
    }
    auto result = (TDest)source;
    if ((TSource)result != source) {
        throw std::runtime_error("float_to_integral: Could not cast floating-point number to integral: " + std::to_string(source));
    }
    return result;
}

}
