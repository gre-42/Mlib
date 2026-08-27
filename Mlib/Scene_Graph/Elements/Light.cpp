#include "Light.hpp"
#include <Mlib/Array/Fixed_Array_Hash.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>

using namespace Mlib;

bool Light::emits_colors() const {
    return (shadow_render_pass == ExternalRenderPassType::NONE) ||
        any(shadow_render_pass & ExternalRenderPassType::LIGHTMAP_EMITS_COLORS_MASK);
}

size_t Light::shading_hash() const {
    return hash_combine(
        ambient,
        diffuse,
        specular,
        fresnel_ambient,
        fog_ambient,
        vp.has_value() ? (int)VisibilityCheck{*vp}.orthographic() : 2,
        shadow_render_pass);
}

void Light::print(std::ostream& ostr) const {
    ostr << "ambient: " << ambient << '\n';
    ostr << "diffuse: " << diffuse << '\n';
    ostr << "specular: " << specular << '\n';
    ostr << "fresnel_ambient: " << fresnel_ambient << '\n';
    ostr << "fog_ambient: " << fog_ambient << '\n';
    ostr << "lightmap_color: " << (int)(lightmap_color != nullptr) << '\n';
    ostr << "lightmap_depth: " << (int)(lightmap_depth != nullptr) << '\n';
    ostr << "vp: " << (int)vp.has_value() << '\n';
    ostr << "shadow_render_pass: " << external_render_pass_type_to_string(shadow_render_pass) << '\n';
    ostr << "emits_colors: " << (int)emits_colors() << '\n';
    ostr << "shading_hash: " << shading_hash() << '\n';
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Light& light) {
    light.print(ostr);
    return ostr;
}
