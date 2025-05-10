#include "Colormap_With_Modifiers.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers_Hash.hpp>
#include <Mlib/Hash.hpp>

using namespace Mlib;

std::partial_ordering ColormapWithModifiers::operator <=> (const ColormapWithModifiers& other) const {
    if (!hash.has_value()) {
        THROW_OR_ABORT("Hash not computed for colormap \"" + *filename + '"');
    }
    if (!other.hash.has_value()) {
        THROW_OR_ABORT("Hash not computed for colormap \"" + *other.filename + '"');
    }
    return *hash <=> *other.hash;
}

bool ColormapWithModifiers::operator == (const ColormapWithModifiers& other) const {
    if (!hash.has_value()) {
        THROW_OR_ABORT("Hash not computed for colormap \"" + *filename + '"');
    }
    if (!other.hash.has_value()) {
        THROW_OR_ABORT("Hash not computed for colormap \"" + *other.filename + '"');
    }
    return *hash == *other.hash;
}

ColormapWithModifiers& ColormapWithModifiers::compute_hash() {
    hash = Mlib::hash_combine(
        filename,
        desaturate,
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
        selected_color,
        selected_color_near,
        selected_color_far,
        color_to_replace,
        replacement_color,
        replacement_tolerance,
        edge_sigma,
        times,
        plus,
        abs,
        invert,
        height_to_normals,
        saturate,
        color_mode,
        alpha_fac,
        mipmap_mode,
        depth_interpolation,
        anisotropic_filtering_level,
        wrap_modes,
        rotate);
    return *this;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const ColormapWithModifiers& t) {
    ostr <<
        "filename: " << *t.filename << '\n' <<
        "desaturate: " << (int)t.desaturate << '\n' <<
        "alpha: " << t.alpha << '\n' <<
        "histogram: " << t.histogram << '\n' <<
        "average: " << t.average << '\n' <<
        "multiply: " << t.multiply << '\n' <<
        "alpha_blend: " << t.alpha_blend << '\n' <<
        "mean_color: " << t.mean_color << '\n' <<
        "lighten: " << t.lighten << '\n' <<
        "lighten_top: " << t.lighten_top << '\n' <<
        "lighten_bottom: " << t.lighten_bottom << '\n' <<
        "color_mode:" << color_mode_to_string(t.color_mode) << '\n' <<
        "rotate:" << t.rotate;
    return ostr;
}
