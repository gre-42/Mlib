#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/String.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

enum class ColorMode {
    UNDEFINED,
    RGB,
    RGBA
};

inline ColorMode color_mode_from_string(const std::string& str) {
    if (str == "rgb") {
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
    case ColorMode::RGB:
        return "rgb";
    case ColorMode::RGBA:
        return "rgba";
    default:
        throw std::runtime_error("Unknown color mode");
    }
}

struct TextureDescriptor {
    std::string color;
    std::string normal;
    ColorMode color_mode = ColorMode::UNDEFINED;
    std::string histogram = "";
    std::string mixed = "";
    size_t overlap_npixels = 5;
    OrderableFixedArray<float, 3> mean_color = {-1.f, -1.f, -1.f};
    std::strong_ordering operator <=> (const TextureDescriptor&) const = default;
};

inline std::ostream& operator << (std::ostream& ostr, const TextureDescriptor& t) {
    ostr <<
        "color: " << t.color << '\n' <<
        "normal: " << t.normal << '\n' <<
        "color_mode: " << color_mode_to_string(t.color_mode) << '\n' <<
        "histogram: " << t.histogram << '\n' <<
        "mixed: " << t.mixed << '\n' <<
        "overlap_npixels: " << t.overlap_npixels << '\n' <<
        "mean_color: " << t.mean_color << '\n';
    return ostr;
}

}
