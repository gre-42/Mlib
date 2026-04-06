
#include "Color_Mode.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

size_t Mlib::max(ColorMode mode) {
    if (any(mode & ColorMode::RGBA)) {
        return 4;
    }
    if (any(mode & ColorMode::RGB)) {
        return 3;
    }
    if (any(mode & ColorMode::GRAYSCALE)) {
        return 1;
    }
    throw std::runtime_error("Unknown color mode: " + std::to_string((int)mode));
}

ColorMode Mlib::color_mode_from_channels(size_t nchannels) {
    switch (nchannels) {
    case 4:
        return ColorMode::RGBA;
    case 3:
        return ColorMode::RGB;
    case 1:
        return ColorMode::GRAYSCALE;
    }
    throw std::runtime_error("Unsupported number of channels: " + std::to_string(nchannels));
}

ColorMode Mlib::color_mode_from_string(const std::string& str) {
    const std::map<std::string, ColorMode> m{
        {"grayscale", ColorMode::GRAYSCALE},
        {"rgb", ColorMode::RGB},
        {"rgba", ColorMode::RGBA}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        throw std::runtime_error("Unknown color mode: \"" + str + '"');
    }
    return it->second;
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
    case ColorMode::AGR_NORMAL:
        return "agr_normal";
    default:
        return "ColorMode(" + std::to_string((int)mode) + ')';
    }
}
