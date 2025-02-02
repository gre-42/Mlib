#pragma once
#include <cmath>
#include <concepts>
#include <string>

namespace Mlib {

template <std::floating_point TDest, std::integral TSource>
constexpr TDest integral_to_float(TSource source) {
    auto result = (TDest)source;
    if ((TSource)result != source) {
        THROW_OR_ABORT("integral_to_float: Could not cast integral to floating-point number: " + std::to_string(source));
    }
    return result;
}

}
