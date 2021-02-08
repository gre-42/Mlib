#include "Material.hpp"

using namespace Mlib;

std::strong_ordering Mlib::operator <=> (const std::vector<BlendMapTexture>& a, const std::vector<BlendMapTexture>& b) {
    if (a.size() < b.size()) {
        return std::strong_ordering::less;
    }
    if (a.size() > b.size()) {
        return std::strong_ordering::greater;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] < b[i]) {
            return std::strong_ordering::less;
        }
        if (a[i] > b[i]) {
            return std::strong_ordering::greater;
        }
    }
    return std::strong_ordering::equal;
}

Material& Material::compute_color_mode() {
    for (auto& t : textures) {
        t.texture_descriptor.color_mode = (blend_mode == BlendMode::OFF)
            ? ColorMode::RGB
            : ColorMode::RGBA;
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
    if (alpha_distances != default_distances) {
        return true;
    }
    for (const auto& t : textures) {
        if (t.distances != default_distances) {
            return true;
        }
    }
    return false;
}

bool Material::fragments_depend_on_normal() const {
    for (const auto& t : textures) {
        if (t.cosines != default_cosines) {
            return true;
        }
    }
    return false;
}

std::partial_ordering Material::operator <=> (const Material&) const = default;
