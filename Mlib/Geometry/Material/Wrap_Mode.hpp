#pragma once
#include <string>

namespace Mlib {

enum class WrapMode {
    REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
};

WrapMode wrap_mode_from_string(const std::string& str);

}
