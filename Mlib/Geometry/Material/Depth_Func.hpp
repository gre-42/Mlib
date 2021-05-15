#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

/**
 * Explicit numbers to facilitate sorting for rendering.
 */
enum class DepthFunc {
    LESS = 0,
    EQUAL = 1,
    LESS_EQUAL = 2
};

inline DepthFunc depth_func_from_string(const std::string& str) {
    if (str == "less") {
        return DepthFunc::LESS;
    } else if (str == "equal") {
        return DepthFunc::EQUAL;
    } else if (str == "less_equal") {
        return DepthFunc::LESS_EQUAL;
    }
    throw std::runtime_error("Unknown depth func: \"" + str + '"');
}

}
