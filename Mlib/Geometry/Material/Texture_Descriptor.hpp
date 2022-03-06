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
    OrderableFixedArray<float, 3> lighten = {0.f, 0.f, 0.f};
    unsigned int anisotropic_filtering_level = 0;
    std::strong_ordering operator <=> (const TextureDescriptor&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(color);
        archive(normal);
        archive(color_mode);
        archive(desaturate);
        archive(histogram);
        archive(mixed);
        archive(overlap_npixels);
        archive(mean_color);
        archive(lighten);
        archive(anisotropic_filtering_level);
    }
};

inline std::ostream& operator << (std::ostream& ostr, const TextureDescriptor& t) {
    ostr <<
        "color: " << t.color << '\n' <<
        "normal: " << t.normal << '\n' <<
        "color_mode: " << color_mode_to_string(t.color_mode) << '\n' <<
        "desaturate: " << (int)t.desaturate << '\n' <<
        "histogram: " << t.histogram << '\n' <<
        "mixed: " << t.mixed << '\n' <<
        "overlap_npixels: " << t.overlap_npixels << '\n' <<
        "mean_color: " << t.mean_color << '\n' <<
        "lighten: " << t.lighten << '\n' <<
        "anisotropic_filtering_level: " << t.anisotropic_filtering_level << '\n';
    return ostr;
}

}
