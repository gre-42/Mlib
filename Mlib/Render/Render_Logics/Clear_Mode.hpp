#pragma once
#include <string>

namespace Mlib {

enum class ClearMode {
    OFF,
    COLOR,
    DEPTH,
    COLOR_AND_DEPTH
};

std::string clear_mode_to_string(ClearMode clear_mode);

ClearMode clear_mode_from_string(const std::string& str);

}
