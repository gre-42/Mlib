#include "Material.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Material& Material::compute_color_mode() {
    for (auto& t : textures_color) {
        if (t.texture_descriptor.color.color_mode == ColorMode::UNDEFINED) {
            t.texture_descriptor.color.color_mode = (blend_mode == BlendMode::OFF)
                ? ColorMode::RGB
                : ColorMode::RGBA;
        }
    }
    return *this;
}

bool Material::has_normalmap() const {
    for (const auto& t : textures_color) {
        if (!t.texture_descriptor.normal.filename.empty()) {
            return true;
        }
    }
    return false;
}

bool Material::fragments_depend_on_distance() const {
    if (alpha_distances != default_linear_distances) {
        return true;
    }
    for (const auto& t : textures_color) {
        if (t.distances != default_linear_distances) {
            return true;
        }
    }
    return false;
}

bool Material::fragments_depend_on_normal() const {
    for (const auto& t : textures_color) {
        if (t.cosines != default_linear_cosines) {
            return true;
        }
    }
    return false;
}

std::string Material::identifier() const {
    if (textures_color.size() > 0) {
        return "color: " + textures_color.front().texture_descriptor.color.filename;
    } else {
        return "<no texture>";
    }
}

const BillboardAtlasInstance& Material::billboard_atlas_instance(uint32_t billboard_id) const {
    if (billboard_id >= billboard_atlas_instances.size()) {
        THROW_OR_ABORT(
            "Billboard ID out of bounds in material \"" + identifier() + "\" (" +
            std::to_string(billboard_id) +
            " >= " +
            std::to_string(billboard_atlas_instances.size()) + ')');
    }
    return billboard_atlas_instances[billboard_id];
}

double Material::max_center_distance(uint32_t billboard_id) const {
    return (billboard_id == UINT32_MAX)
        ? center_distances(1)
        : billboard_atlas_instance(billboard_id).max_center_distance;
}

ExternalRenderPassType Material::get_occluder_pass(uint32_t billboard_id) const {
    return (billboard_id == UINT32_MAX)
        ? occluder_pass
        : billboard_atlas_instance(billboard_id).occluder_pass;
}
