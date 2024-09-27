#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <concepts>
#include <string>

namespace Mlib {

template <std::integral TDest, std::floating_point TSource>
TDest float_to_integral(TSource source) {
    if (!std::isfinite(source)) {
        THROW_OR_ABORT("float_to_integral: Floating-point number is not finite");
    }
    auto result = (TDest)source;
    if ((TSource)result != source) {
        THROW_OR_ABORT("float_to_integral: Could not cast floating-point number to integral: " + std::to_string(source));
    }
    return result;
}

}
