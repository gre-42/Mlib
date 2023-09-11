#pragma once
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

struct ColormapWithModifiers {
    std::string filename;
    ColorMode color_mode = ColorMode::UNDEFINED;
    bool desaturate = false;
    std::string histogram = "";
    std::string average = "";
    std::string multiply = "";
    OrderableFixedArray<float, 3> mean_color = {-1.f, -1.f, -1.f};
    OrderableFixedArray<float, 3> lighten = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_top = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_bottom = {0.f, 0.f, 0.f};
    std::partial_ordering operator <=> (const ColormapWithModifiers&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(filename);
        archive(color_mode);
        archive(desaturate);
        archive(histogram);
        archive(average);
        archive(multiply);
        archive(mean_color);
        archive(lighten);
        archive(lighten_top);
        archive(lighten_bottom);
    }
};

std::ostream& operator << (std::ostream& ostr, const ColormapWithModifiers& t);

}
