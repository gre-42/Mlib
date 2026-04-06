
#include "Infer_Shader_Properties.hpp"
#include <Mlib/Geometry/Mesh/Mesh_Meta.hpp>

using namespace Mlib;

bool Mlib::get_has_per_instance_continuous_texture_layer(const MeshMeta& mesh_meta) {
    if (mesh_meta.material.textures_color.empty()) {
        throw std::runtime_error("Array \"" + mesh_meta.name.full_name() + "\" has no textures");
    }
    return (mesh_meta.material.textures_color[0].texture_descriptor.color.depth_interpolation == InterpolationMode::LINEAR);
}

bool Mlib::get_has_discrete_atlas_texture_layer(const MeshMeta& mesh_meta) {
    for (const auto& x : mesh_meta.material.billboard_atlas_instances) {
        if (x.texture_layer > 0) {
            return true;
        }
    }
    return false;
}
