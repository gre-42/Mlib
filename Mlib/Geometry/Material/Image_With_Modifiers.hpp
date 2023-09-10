#pragma once
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {


struct ImageWithModifiers {
    std::string filename;
    ColorMode color_mode = ColorMode::UNDEFINED;
    bool desaturate = false;
    std::string histogram = "";
    std::string mixed = "";
    size_t overlap_npixels = 5;
    OrderableFixedArray<float, 3> mean_color = {-1.f, -1.f, -1.f};
    OrderableFixedArray<float, 3> lighten = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_top = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_bottom = {0.f, 0.f, 0.f};
    std::partial_ordering operator <=> (const ImageWithModifiers&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(filename);
        archive(color_mode);
        archive(desaturate);
        archive(histogram);
        archive(mixed);
        archive(overlap_npixels);
        archive(mean_color);
        archive(lighten);
        archive(lighten_top);
        archive(lighten_bottom);
    }
};

inline std::ostream& operator << (std::ostream& ostr, const ImageWithModifiers& t) {
    ostr <<
        "desaturate: " << (int)t.desaturate << '\n' <<
        "histogram: " << t.histogram << '\n' <<
        "mixed: " << t.mixed << '\n' <<
        "overlap_npixels: " << t.overlap_npixels << '\n' <<
        "mean_color: " << t.mean_color << '\n' <<
        "lighten: " << t.lighten << '\n' <<
        "lighten_top: " << t.lighten_top << '\n' <<
        "lighten_bottom: " << t.lighten_bottom << '\n';
    return ostr;
}

}
