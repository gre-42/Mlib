
#include "Colormap_With_Modifiers.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers_Hash.hpp>
#include <Mlib/Hashing/Hash.hpp>

using namespace Mlib;

std::partial_ordering ColormapWithModifiers::operator <=> (const ColormapWithModifiers& other) const {
    if (!hash.has_value()) {
        throw std::runtime_error("Hash not computed for colormap \"" + filename.string() + '"');
    }
    if (!other.hash.has_value()) {
        throw std::runtime_error("Hash not computed for colormap \"" + other.filename.string() + '"');
    }
    return *hash <=> *other.hash;
}

bool ColormapWithModifiers::operator == (const ColormapWithModifiers& other) const {
    if (!hash.has_value()) {
        throw std::runtime_error("Hash not computed for colormap \"" + filename.string() + '"');
    }
    if (!other.hash.has_value()) {
        throw std::runtime_error("Hash not computed for colormap \"" + other.filename.string() + '"');
    }
    return *hash == *other.hash;
}

ColormapWithModifiers& ColormapWithModifiers::compute_hash() {
    hash = Mlib::hash_combine(
        filename,
        chrominance,
        desaturate,
        desaturation_exponent,
        alpha,
        histogram,
        average,
        multiply,
        alpha_blend,
        mean_color,
        lighten,
        lighten_left,
        lighten_right,
        lighten_top,
        lighten_bottom,
        color_to_replace,
        replacement_color,
        replacement_tolerance,
        selected_color,
        selected_color_near,
        selected_color_far,
        edge_sigma,
        times,
        plus,
        abs,
        invert,
        height_to_normals,
        saturate,
        multiply_with_alpha,
        color_mode,
        alpha_fac,
        alpha_exponent,
        mipmap_mode,
        magnifying_interpolation_mode,
        depth_interpolation,
        anisotropic_filtering_level,
        wrap_modes,
        border_color,
        rotate);
    return *this;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const ColormapWithModifiers& t) {
    ostr <<
        "filename: " << t.filename << '\n' <<
        "chrominance: " << t.chrominance << '\n' <<
        "desaturate: " << (int)t.desaturate << '\n' <<
        "desaturation_exponent: " << (int)t.desaturation_exponent << '\n' <<
        "alpha: " << t.alpha << '\n' <<
        "histogram: " << t.histogram << '\n' <<
        "average: " << t.average << '\n' <<
        "multiply: " << t.multiply << '\n' <<
        "alpha_blend: " << t.alpha_blend << '\n' <<
        "mean_color: " << t.mean_color << '\n' <<
        "lighten: " << t.lighten << '\n' <<
        "lighten_left: " << t.lighten_left << '\n' <<
        "lighten_right: " << t.lighten_right << '\n' <<
        "lighten_top: " << t.lighten_top << '\n' <<
        "lighten_bottom: " << t.lighten_bottom << '\n' <<
        "color_to_replace: " << t.color_to_replace << '\n' <<
        "replacement_color: " << t.replacement_color << '\n' <<
        "replacement_tolerance: " << t.replacement_tolerance << '\n' <<
        "selected_color: " << t.selected_color << '\n' <<
        "selected_color_near: " << t.selected_color_near << '\n' <<
        "selected_color_far: " << t.selected_color_far << '\n' <<
        "edge_sigma: " << t.edge_sigma << '\n' <<
        "times: " << t.times << '\n' <<
        "plus: " << t.plus << '\n' <<
        "abs: " << t.abs << '\n' <<
        "invert: " << t.invert << '\n' <<
        "height_to_normals: " << t.height_to_normals << '\n' <<
        "saturate: " << t.saturate << '\n' <<
        "multiply_with_alpha: " << t.multiply_with_alpha << '\n' <<
        "color_mode: " << color_mode_to_string(t.color_mode) << '\n' <<
        "alpha_fac: " << t.alpha_fac << '\n' <<
        "alpha_exponent: " << t.alpha_exponent << '\n' <<
        "mipmap_mode: " << mipmap_mode_to_string(t.mipmap_mode) << '\n' <<
        "magnifying_interpolation_mode: " << interpolation_mode_to_string(t.magnifying_interpolation_mode) << '\n' <<
        "depth_interpolation: " << interpolation_mode_to_string(t.depth_interpolation) << '\n' <<
        "anisotropic_filtering_level: " << t.anisotropic_filtering_level << '\n' <<
        "wrap_modes: " << wrap_mode_to_string(t.wrap_modes(0)) << ", " << wrap_mode_to_string(t.wrap_modes(1)) << '\n' <<
        "border_color: " << t.border_color << '\n' <<
        "rotate: " << t.rotate;
    return ostr;
}
