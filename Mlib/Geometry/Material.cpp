#include "Material.hpp"

using namespace Mlib;

Material& Material::compute_color_mode() {
    for (auto& t : textures) {
        if (t.texture_descriptor.color_mode == ColorMode::UNDEFINED) {
            t.texture_descriptor.color_mode = (blend_mode == BlendMode::OFF)
                ? ColorMode::RGB
                : ColorMode::RGBA;
        }
    }
    return *this;
}

bool Material::has_normalmap() const {
    for (const auto& t : textures) {
        if (!t.texture_descriptor.normal.empty()) {
            return true;
        }
    }
    return false;
}

bool Material::fragments_depend_on_distance() const {
    if (alpha_distances != default_linear_distances) {
        return true;
    }
    for (const auto& t : textures) {
        if (t.distances != default_linear_distances) {
            return true;
        }
    }
    return false;
}

bool Material::fragments_depend_on_normal() const {
    for (const auto& t : textures) {
        if (t.cosines != default_linear_cosines) {
            return true;
        }
    }
    return false;
}

std::string Material::identifier() const {
    if (textures.size() > 0) {
        return "color: " + textures.front().texture_descriptor.color;
    } else {
        return "<no texture>";
    }
}

const BillboardAtlasInstance& Material::billboard_atlas_instance(uint32_t billboard_id) const {
    if (billboard_id >= billboard_atlas_instances.size()) {
        throw std::runtime_error(
            "Billboard ID out of bounds in material \"" + identifier() + "\" (" +
            std::to_string(billboard_id) +
            " >= " +
            std::to_string(billboard_atlas_instances.size()) + ')');
    }
    return billboard_atlas_instances[billboard_id];
}
