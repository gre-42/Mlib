#include "Colormap_With_Modifiers_Hash.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Hash.hpp>

using namespace Mlib;

std::size_t std::hash<Mlib::ColormapWithModifiers>::operator() (const Mlib::ColormapWithModifiers& k) const {
    return Mlib::hash_combine(
        k.filename,
        k.desaturate,
        k.alpha,
        k.histogram,
        k.average,
        k.multiply,
        k.alpha_blend,
        k.mean_color,
        k.lighten,
        k.lighten_left,
        k.lighten_right,
        k.lighten_top,
        k.lighten_bottom,
        k.selected_color,
        k.selected_color_near,
        k.selected_color_far,
        k.edge_sigma,
        k.times,
        k.plus,
        k.abs,
        k.invert,
        k.color_mode,
        k.alpha_fac,
        k.mipmap_mode,
        k.depth_interpolation,
        k.anisotropic_filtering_level,
        k.wrap_modes);
}
