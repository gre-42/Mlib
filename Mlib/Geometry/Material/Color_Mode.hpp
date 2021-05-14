#pragma once

namespace Mlib {

enum class ColorMode {
    UNDEFINED,
    GRAYSCALE = 1,
    RGB = 3,
    RGBA = 4
};

inline ColorMode color_mode_from_string(const std::string& str) {
    if (str == "grayscale") {
        return ColorMode::GRAYSCALE;
    } else if (str == "rgb") {
        return ColorMode::RGB;
    } else if (str == "rgba") {
        return ColorMode::RGBA;
    }
    throw std::runtime_error("Unknown color mode");
}

inline std::string color_mode_to_string(const ColorMode& mode) {
    switch (mode) {
    case ColorMode::UNDEFINED:
        return "undefined";
    case ColorMode::GRAYSCALE:
        return "grayscale";
    case ColorMode::RGB:
        return "rgb";
    case ColorMode::RGBA:
        return "rgba";
    default:
        throw std::runtime_error("Unknown color mode");
    }
}

}
