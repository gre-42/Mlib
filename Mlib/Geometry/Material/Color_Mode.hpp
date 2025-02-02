#pragma once
#include <string>

namespace Mlib {

enum class ColorMode {
    NONE = 0,
    UNDEFINED = 1 << 0,
    GRAYSCALE = 1 << 1,
    RGB = 1 << 2,
    RGBA = 1 << 3,
    AGR_NORMAL = 1 << 4
};

inline ColorMode operator & (ColorMode a, ColorMode b) {
    return (ColorMode)((int)a & (int)b);
}

inline ColorMode operator | (ColorMode a, ColorMode b) {
    return (ColorMode)((int)a | (int)b);
}

inline ColorMode& operator |= (ColorMode& a, ColorMode b) {
    (int&)a |= (int)b;
    return a;
}

inline bool any(ColorMode a) {
    return a != ColorMode::NONE;
}

size_t max(ColorMode mode);
ColorMode color_mode_from_channels(size_t nchannels);
ColorMode color_mode_from_string(const std::string& str);
std::string color_mode_to_string(const ColorMode& mode);

}
