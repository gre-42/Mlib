#pragma once
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

struct TextureDescriptor {
    std::string color;
    std::string normal;
    ColorMode color_mode = ColorMode::UNDEFINED;
    bool desaturate = false;
    std::string histogram = "";
    std::string mixed = "";
    size_t overlap_npixels = 5;
    OrderableFixedArray<float, 3> mean_color = {-1.f, -1.f, -1.f};
    unsigned int anisotropic_filtering_level = 0;
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
