#include "Color_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ColorMode Mlib::color_mode_from_string(const std::string& str) {
    if (str == "grayscale") {
        return ColorMode::GRAYSCALE;
    } else if (str == "rgb") {
        return ColorMode::RGB;
    } else if (str == "rgba") {
        return ColorMode::RGBA;
    }
    THROW_OR_ABORT("Unknown color mode: \"" + str + '"');
}

std::string Mlib::color_mode_to_string(const ColorMode& mode) {
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
        THROW_OR_ABORT("Unknown color mode");
    }
}
