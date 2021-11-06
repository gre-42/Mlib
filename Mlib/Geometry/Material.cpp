#include "Material.hpp"

using namespace Mlib;

template <class TData>
static std::strong_ordering compare_vectors(const std::vector<TData>& a, const std::vector<TData>& b) {
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

std::strong_ordering Mlib::operator <=> (const std::vector<BlendMapTexture>& a, const std::vector<BlendMapTexture>& b) {
    return compare_vectors(a, b);
}

std::strong_ordering Mlib::operator <=> (const std::vector<BillboardAtlasInstance>& a, const std::vector<BillboardAtlasInstance>& b) {
    return compare_vectors(a, b);
}

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
