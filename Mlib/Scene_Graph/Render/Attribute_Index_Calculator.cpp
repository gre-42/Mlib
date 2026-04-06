#include "Attribute_Index_Calculator.hpp"
#include <Mlib/Geometry/Material_Features.hpp>
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Instance_Buffers.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Data.hpp>
#include <ostream>

using namespace Mlib;

AttributeIndices AttributeIndexCalculator::build() const {
    AttributeIndices result;
    result.idx_position = 0;
    result.idx_color = result.idx_position + has_position;
    result.idx_normal = result.idx_color + has_color;
    result.idx_tangent = result.idx_normal + has_normal;
    result.idx_instance_attrs = result.idx_tangent + has_tangent;
    result.idx_rotation_quaternion = result.idx_instance_attrs + has_instance_attrs;
    result.idx_billboard_ids = result.idx_rotation_quaternion + has_rotation_quaternion;
    result.idx_bone_indices = result.idx_billboard_ids + has_billboard_ids;
    result.idx_bone_weights = result.idx_bone_indices + has_bone_indices;
    result.idx_texture_layer = result.idx_bone_weights + has_bone_weights;
    result.idx_interior_mapping_bottom_left = result.idx_texture_layer + has_texture_layer;
    result.idx_interior_mapping_uvmap = result.idx_interior_mapping_bottom_left + has_interior_mapping_bottom_left;
    result.idx_uv_0 = result.idx_interior_mapping_uvmap + has_interior_mapping_multiplier;
    result.idx_uv_1 = result.idx_uv_0 + 1;
    result.uv_count = integral_cast<uint32_t>(nuvs);
    result.idx_cweight_0 = result.idx_uv_0 + result.uv_count;
    result.cweight_count = integral_cast<uint32_t>(ncweights);
    result.idx_alpha = result.idx_cweight_0 + result.cweight_count;
    return result;
}

AttributeIndexCalculator Mlib::get_attribute_index_calculator(const IGpuVertexArray& gva)
{
    auto vertices = gva.vertices();
    auto instances = gva.instances();
    const auto& meta = vertices->mesh_meta();
    return AttributeIndexCalculator{
        .has_position = true,
        .has_color = true,
        .has_normal =
            meta.material.reorient_uv0 ||
            !meta.material.shading.diffuse.all_equal(0) ||
            !meta.material.shading.specular.all_equal(0) ||
            (!meta.material.shading.reflectance.all_equal(0.f) && !meta.material.reflect_only_y && !meta.material.reflection_map.empty()) ||
            (meta.material.shading.fresnel.reflectance.exponent != 0.f) ||
            fragments_depend_on_normal(meta.material.textures_color) ||
            !meta.material.interior_textures.empty(),
        .has_tangent = has_normalmap(meta.material.textures_color) || !meta.material.interior_textures.empty(),
        .has_instance_attrs = instances != nullptr,
        .has_rotation_quaternion = (instances != nullptr) && (meta.material.transformation_mode == TransformationMode::ALL),
        .has_billboard_ids = !meta.material.billboard_atlas_instances.empty(),
        .has_bone_indices = vertices->has_bone_indices(),
        .has_bone_weights = vertices->has_bone_indices(),
        .has_texture_layer =
            ((instances != nullptr) && instances->has_continuous_texture_layer()) ||
            vertices->has_continuous_triangle_texture_layers() ||
            vertices->has_discrete_triangle_texture_layers(),
        .has_interior_mapping_bottom_left = !meta.material.interior_textures.empty(),
        .has_interior_mapping_multiplier = !meta.material.interior_textures.empty(),
        .nuvs = vertices->nuvs(),             // cva.uv1.size() + 1,
        .ncweights = vertices->ncweights(),   // cva.cweight.size(),
        .has_alpha = vertices->has_alpha()    // !cva.alpha.empty()
    };
}

std::ostream& Mlib::operator << (
    std::ostream& ostr,
    const AttributeIndexCalculator& attr_idc)
{
    ostr << "has_position: " << (int)attr_idc.has_position << '\n';
    ostr << "has_color: " << (int)attr_idc.has_color << '\n';
    ostr << "has_normal: " << (int)attr_idc.has_normal << '\n';
    ostr << "has_tangent: " << (int)attr_idc.has_tangent << '\n';
    ostr << "has_instance_attrs: " << (int)attr_idc.has_instance_attrs << '\n';
    ostr << "has_rotation_quaternion: " << (int)attr_idc.has_rotation_quaternion << '\n';
    ostr << "has_billboard_ids: " << (int)attr_idc.has_billboard_ids << '\n';
    ostr << "has_bone_indices: " << (int)attr_idc.has_bone_indices << '\n';
    ostr << "has_bone_weights: " << (int)attr_idc.has_bone_weights << '\n';
    ostr << "has_texture_layer: " << (int)attr_idc.has_texture_layer << '\n';
    ostr << "has_interior_mapping_bottom_left: " << (int)attr_idc.has_interior_mapping_bottom_left << '\n';
    ostr << "has_interior_mapping_multiplier: " << (int)attr_idc.has_interior_mapping_multiplier << '\n';
    ostr << "nuvs: " << attr_idc.nuvs << '\n';
    ostr << "ncweights: " << attr_idc.ncweights << '\n';
    ostr << "has_alpha: " << (int)attr_idc.has_alpha << '\n';
    return ostr;
}
