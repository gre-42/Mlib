#pragma once
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

struct ColormapWithModifiers {
    std::string filename;
    bool desaturate = false;
    std::string alpha = "";
    std::string histogram = "";
    std::string average = "";
    std::string multiply = "";
    std::string alpha_blend = "";
    OrderableFixedArray<float, 3> mean_color = {-1.f, -1.f, -1.f};
    OrderableFixedArray<float, 3> lighten = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_left = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_right = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_top = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> lighten_bottom = {0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> selected_color = {-1.f, -1.f, -1.f};
    float selected_color_near = 0;
    float selected_color_far = INFINITY;
    float times = 1.f;
    float plus = 0.f;
    std::partial_ordering operator <=> (const ColormapWithModifiers&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(filename);
        archive(desaturate);
        archive(alpha);
        archive(histogram);
        archive(average);
        archive(multiply);
        archive(alpha_blend);
        archive(mean_color);
        archive(lighten);
        archive(lighten_top);
        archive(lighten_bottom);
        archive(selected_color);
        archive(selected_color_near);
        archive(selected_color_far);
        archive(times);
        archive(plus);
    }
};

std::ostream& operator << (std::ostream& ostr, const ColormapWithModifiers& t);

}
