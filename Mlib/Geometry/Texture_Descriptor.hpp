#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
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

struct TextureDescriptor {
    std::string color;
    ColorMode color_mode = ColorMode::UNDEFINED;
    std::string histogram = "";
    std::string mixed = "";
    size_t overlap_npixels = 5;
    OrderableFixedArray<float, 3> mean_color = {-1, -1, -1};
    std::strong_ordering operator <=> (const TextureDescriptor&) const = default;
};

}
