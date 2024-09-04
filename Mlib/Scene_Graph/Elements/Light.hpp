#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <string>

namespace Mlib {

enum class ExternalRenderPassType;

struct Light {
    FixedArray<float, 3> ambient{1.f, 1.f, 1.f};
    FixedArray<float, 3> diffuse{1.f, 1.f, 1.f};
    FixedArray<float, 3> specular{1.f, 1.f, 1.f};
    FixedArray<float, 3> fresnel_ambient{1.f, 1.f, 1.f};
    FixedArray<float, 3> fog_ambient{1.f, 1.f, 1.f};
    ColormapWithModifiers lightmap_color = ColormapWithModifiers{}.compute_hash();
    ColormapWithModifiers lightmap_depth = ColormapWithModifiers{}.compute_hash();
    ExternalRenderPassType shadow_render_pass;
    bool emits_colors() const;
};

}
