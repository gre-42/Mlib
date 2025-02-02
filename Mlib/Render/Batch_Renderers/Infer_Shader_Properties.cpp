#include "Infer_Shader_Properties.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

bool Mlib::get_has_per_instance_continuous_texture_layer(const ColoredVertexArray<float>& cva) {
    if (cva.material.textures_color.empty()) {
        THROW_OR_ABORT("Array \"" + cva.name + "\" has no textures");
    }
    return (cva.material.textures_color[0].texture_descriptor.color.depth_interpolation == InterpolationMode::LINEAR);
}

bool Mlib::get_has_discrete_atlas_texture_layer(const ColoredVertexArray<float>& cva) {
    for (const auto& x : cva.material.billboard_atlas_instances) {
        if (x.texture_layer > 0) {
            return true;
        }
    }
    return false;
}
