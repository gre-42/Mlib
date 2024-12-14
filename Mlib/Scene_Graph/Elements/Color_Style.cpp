#include "Color_Style.hpp"
#include <Mlib/Hash.hpp>

using namespace Mlib;

void ColorStyle::insert(const ColorStyle& other) {
    if (!other.emissive.all_equal(-1.f)) {
        this->emissive = other.emissive;
    }
    if (!other.ambient.all_equal(-1.f)) {
        this->ambient = other.ambient;
    }
    if (!other.diffuse.all_equal(-1.f)) {
        this->diffuse = other.diffuse;
    }
    if (!other.specular.all_equal(-1.f)) {
        this->specular = other.specular;
    }
    if (!other.fresnel_ambient.all_equal(-1.f)) {
        this->fresnel_ambient = other.fresnel_ambient;
    }
    if (other.fresnel.exponent != -1.f) {
        this->fresnel = other.fresnel;
    }
    if (other.reflection_strength != -1.f) {
        this->reflection_strength = other.reflection_strength;
    }
    for (const auto& [key, value] : other.reflection_maps) {
        this->reflection_maps[key] = value;
    }
}

bool ColorStyle::matches(const VariableAndHash<std::string>& name) const {
    return matches_.get(name);
}

size_t ColorStyle::get_hash() const {
    std::scoped_lock lock{ hash_mutex_ };
    return hash_.get();
}

void ColorStyle::update_hash() {
    std::scoped_lock lock{ hash_mutex_ };
    hash_.reset();
    compute_hash();
}

ColorStyle& ColorStyle::compute_hash() {
    Hasher hasher{ 0xc0febabe };
    hasher.combine(
        emissive,
        ambient,
        diffuse,
        specular,
        specular_exponent,
        fresnel_ambient,
        fresnel,
        reflection_strength);
    hasher.combine(reflection_maps.size());
    for (const auto& [k, v] : reflection_maps) {
        hasher.combine(k, v);
    }
    hash_ = hasher;
    return *this;
}
