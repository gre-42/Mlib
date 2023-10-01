#pragma once
#include <string>

namespace Mlib {

enum class ColorMode {
    UNDEFINED = 0,
    GRAYSCALE = 1,
    RGB = 3,
    RGBA = 4
};

ColorMode color_mode_from_string(const std::string& str);

std::string color_mode_to_string(const ColorMode& mode);

}
