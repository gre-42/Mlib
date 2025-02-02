#pragma once
#include <Mlib/Cached_Hash.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

struct ColormapWithModifiers {
    VariableAndHash<std::string> filename;
    float desaturate = 0.f;
    std::string alpha = "";
    std::string histogram = "";
    std::string average = "";
    std::string multiply = "";
    std::string alpha_blend = "";
    OrderableFixedArray<float, 3> mean_color = { -1.f, -1.f, -1.f };
    OrderableFixedArray<float, 3> lighten = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 3> lighten_left = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 3> lighten_right = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 3> lighten_top = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 3> lighten_bottom = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 3> color_to_replace = { -1.f, -1.f, -1.f };
    OrderableFixedArray<float, 3> replacement_color = { -1.f, -1.f, -1.f };
    float replacement_tolerance = 0;
    OrderableFixedArray<float, 3> selected_color = { -1.f, -1.f, -1.f };
    float selected_color_near = 0;
    float selected_color_far = INFINITY;
    float edge_sigma = 0.f;
    float times = 1.f;
    float plus = 0.f;
    bool abs = false;
    bool invert = false;
    bool height_to_normals = false;
    bool saturate = false;
    ColorMode color_mode = ColorMode::UNDEFINED;
    float alpha_fac = 1.f;
    MipmapMode mipmap_mode = MipmapMode::NO_MIPMAPS;
    InterpolationMode depth_interpolation = InterpolationMode::NEAREST;
    unsigned int anisotropic_filtering_level = 0;
    OrderableFixedArray<WrapMode, 2> wrap_modes = { WrapMode::REPEAT, WrapMode::REPEAT };
    CachedHash hash;
    ColormapWithModifiers& compute_hash();
    std::partial_ordering operator <=> (const ColormapWithModifiers& other) const;
    bool operator == (const ColormapWithModifiers& other) const;
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
        archive(lighten_left);
        archive(lighten_right);
        archive(lighten_top);
        archive(lighten_bottom);
        archive(selected_color);
        archive(selected_color_near);
        archive(selected_color_far);
        archive(color_to_replace);
        archive(replacement_color);
        archive(replacement_tolerance);
        archive(edge_sigma);
        archive(times);
        archive(plus);
        archive(abs);
        archive(invert);
        archive(height_to_normals);
        archive(saturate);
        archive(color_mode);
        archive(alpha_fac);
        archive(mipmap_mode);
        archive(depth_interpolation);
        archive(anisotropic_filtering_level);
        archive(wrap_modes);
        archive(hash);
    }
};

std::ostream& operator << (std::ostream& ostr, const ColormapWithModifiers& t);

}
