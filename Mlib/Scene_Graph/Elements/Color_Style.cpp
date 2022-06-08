#include "Color_Style.hpp"

using namespace Mlib;

void ColorStyle::insert(const ColorStyle& other) {
    if (!all(other.emissivity == -1.f)) {
        this->emissivity = other.emissivity;
    }
    if (!all(other.ambience == -1.f)) {
        this->ambience = other.ambience;
    }
    if (!all(other.diffusivity == -1.f)) {
        this->diffusivity = other.diffusivity;
    }
    if (!all(other.specularity == -1.f)) {
        this->specularity = other.specularity;
    }
    if (other.reflection_strength != -1.f) {
        this->reflection_strength = other.reflection_strength;
    }
    for (const auto& [key, value] : other.reflection_maps) {
        this->reflection_maps[key] = value;
    }
}
