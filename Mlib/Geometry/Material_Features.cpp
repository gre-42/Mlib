#include "Material_Features.hpp"
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>

using namespace Mlib;


bool Mlib::has_normalmap(const std::vector<BlendMapTexture>& textures_color) {
    for (const auto& t : textures_color) {
        if (!t.texture_descriptor.normal.filename->empty()) {
            return true;
        }
    }
    return false;
}

bool Mlib::fragments_depend_on_distance(
    const FixedArray<float, 2>& fog_distances,
    const FixedArray<float, 4>& alpha_distances,
    const std::vector<BlendMapTextureAndId>& textures,
    const std::map<ColormapWithModifiers, size_t>& texture_ids_color)
{
    if (any(fog_distances != default_step_distances)) {
        return true;
    }
    if (any(alpha_distances != default_linear_distances)) {
        return true;
    }
    for (const auto& [_, i] : texture_ids_color) {
        if (textures.at(i).ops->distances != default_linear_distances) {
            return true;
        }
    }
    return false;
}

bool Mlib::fragments_depend_on_normal(const std::vector<BlendMapTexture>& textures_color)
{
    for (const auto& t : textures_color) {
        if (t.cosines != default_linear_cosines) {
            return true;
        }
    }
    return false;
}

bool Mlib::has_horizontal_detailmap(
    const std::vector<BlendMapTextureAndId>& textures,
    const std::map<ColormapWithModifiers, size_t>& texture_ids)
{
    for (const auto& [_, i] : texture_ids) {
        if (textures.at(i).ops->uv_source == BlendMapUvSource::HORIZONTAL) {
            return true;
        }
    }
    return false;
}
