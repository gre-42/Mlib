#pragma once
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

DepthFunc depth_func_from_string(const std::string& str);

}
