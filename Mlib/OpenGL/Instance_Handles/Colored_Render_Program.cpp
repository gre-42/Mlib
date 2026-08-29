#include "Colored_Render_Program.hpp"
#include <Mlib/Strings/Join.hpp>
#include <ostream>

using namespace Mlib;

void RenderProgramIdentifier::print(std::ostream& ostr) const {
    ostr << "attr_idc: " << attr_idc << '\n';
    ostr << "is_lightmap_blobs_render_pass: " << (int)is_lightmap_blobs_render_pass << '\n';
    ostr << "is_lightmap_color_render_pass: " << (int)is_lightmap_color_render_pass << '\n';
    ostr << "skidmarks_hash: " << skidmarks_hash << '\n';
    ostr << "nbones: " << nbones << '\n';
    ostr << "blend_mode: " << uint32_t(blend_mode) << '\n';
    ostr << "alpha_distances: " << alpha_distances << '\n';
    ostr << "fog_distances: " << fog_distances << '\n';
    ostr << "fog_emissive: " << fog_emissive << '\n';
    ostr << "ntextures_color: " << ntextures_color << '\n';
    ostr << "ntextures_normal: " << ntextures_normal << '\n';
    ostr << "ntextures_alpha: " << ntextures_alpha << '\n';
    ostr << "has_dynamic_emissive: " << (int)has_dynamic_emissive << '\n';
    ostr << "lightmap_indices_color: " << Mlib::join(" ", lightmap_indices_color, [](const auto& i){ return std::to_string(i); }) << '\n';
    ostr << "lightmap_indices_depth: " << Mlib::join(" ", lightmap_indices_depth, [](const auto& i){ return std::to_string(i); }) << '\n';
    ostr << "has_specularmap: " << (int)has_specularmap << '\n';
    ostr << "reflectance: " << reflectance << '\n';
    ostr << "reflect_only_y: " << (int)reflect_only_y << '\n';
    ostr << "ntextures_reflection: " << ntextures_reflection << '\n';
    ostr << "ntextures_dirt: " << ntextures_dirt << '\n';
    ostr << "interior_texture_set: " << (uint32_t)interior_texture_set << '\n';
    ostr << "facade_inner_size: " << facade_inner_size << '\n';
    ostr << "interior_size: " << interior_size << '\n';
    ostr << "nuv_indices: " << nuv_indices << '\n';
    ostr << "ncweights: " << ncweights << '\n';
    ostr << "has_alpha: " << (int)has_alpha << '\n';
    ostr << "continuous_layer_x: " << Mlib::join(" ", continuous_layer_x, [](const auto& i){ return std::to_string(i); }) << '\n';
    ostr << "continuous_layer_y: " << Mlib::join(" ", continuous_layer_y, [](const auto& i){ return std::to_string(i); }) << '\n';
    ostr << "has_horizontal_detailmap: " << (int)has_horizontal_detailmap << '\n';
    ostr << "dirt_color_mode: " << color_mode_to_string(dirt_color_mode) << '\n';
    ostr << "has_instances: " << (int)has_instances << '\n';
    ostr << "has_flat: " << (int)has_flat << '\n';
    ostr << "has_lookat: " << (int)has_lookat << '\n';
    ostr << "has_yangle: " << (int)has_yangle << '\n';
    ostr << "has_rotation_quaternion: " << (int)has_rotation_quaternion << '\n';
    ostr << "has_uv_offset_u: " << (int)has_uv_offset_u << '\n';
    ostr << "texture_layer_properties: " << (uint32_t)texture_layer_properties << '\n';
    ostr << "nbillboard_ids: " << nbillboard_ids << '\n';
    ostr << "reorient_normals: " << (int)reorient_normals << '\n';
    ostr << "reorient_uv0: " << (int)reorient_uv0 << '\n';
    ostr << "emissive: " << emissive << '\n';
    ostr << "ambient: " << ambient << '\n';
    ostr << "diffuse: " << diffuse << '\n';
    ostr << "specular: " << specular << '\n';
    ostr << "specular_exponent: " << specular_exponent << '\n';
    ostr << "fresnel_emissive: " << fresnel_emissive << '\n';
    ostr << "<fresnel>" << '\n';
    ostr << "alpha: " << alpha << '\n';
    ostr << "orthographic: " << (int)orthographic << '\n';
    ostr << "fragments_depend_on_distance: " << (int)fragments_depend_on_distance << '\n';
    ostr << "fragments_depend_on_normal: " << (int)fragments_depend_on_normal << '\n';
    ostr << "dirtmap_offset: " << dirtmap_offset << '\n';
    ostr << "dirtmap_discreteness: " << dirtmap_discreteness << '\n';
    ostr << "dirt_scale: " << dirt_scale << '\n';
    ostr << "texture_modifiers_hash: " << texture_modifiers_hash << '\n';
    ostr << "lights_hash: " << lights_hash << '\n';
};

std::ostream& Mlib::operator << (std::ostream& ostr, const RenderProgramIdentifier& id) {
    id.print(ostr);
    return ostr;
}
