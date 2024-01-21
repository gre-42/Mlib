#include "Color_Style.hpp"

using namespace Mlib;

void ColorStyle::insert(const ColorStyle& other) {
    if (!all(other.emissive == -1.f)) {
        this->emissive = other.emissive;
    }
    if (!all(other.ambient == -1.f)) {
        this->ambient = other.ambient;
    }
    if (!all(other.diffuse == -1.f)) {
        this->diffuse = other.diffuse;
    }
    if (!all(other.specular == -1.f)) {
        this->specular = other.specular;
    }
    if (!all(other.fresnel_ambient == -1.f)) {
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
