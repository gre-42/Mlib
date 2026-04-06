#pragma once
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace Mlib {

static const uint16_t uint16_nan = (1 << 8);

inline uint16_t uint16_from_float(float grayscale) {
    if (std::isnan(grayscale)) {
        return uint16_nan;
    }
    // consider using grayscale.clip(0, 1) if this fails
    if (grayscale < 0) {
        throw std::runtime_error("uint16_from_float received " + std::to_string(grayscale) + "<0");
    }
    if (grayscale > 1) {
        throw std::runtime_error("uint16_from_float received " + std::to_string(grayscale) + ">1");
    }
    return (uint16_t)std::round(grayscale * UINT16_MAX);
}

}
