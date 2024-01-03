#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <map>

namespace Mlib {

/**
 * Don't forget to update the "insert" function when adding new fields.
 */
struct ColorStyle {
    Mlib::regex selector;
    FixedArray<float, 3> emissivity{-1.f, -1.f, -1.f};
    FixedArray<float, 3> ambience{-1.f, -1.f, -1.f};
    FixedArray<float, 3> diffusivity{-1.f, -1.f, -1.f};
    FixedArray<float, 3> specularity{-1.f, -1.f, -1.f};
    float specular_exponent = -1.f;
    FixedArray<float, 3> fresnel_ambience{-1.f, -1.f, -1.f};
    FresnelReflectance fresnel{
        .min = -1.f,
        .max = -1.f,
        .exponent = -1.f
    };
    std::map<std::string, std::string> reflection_maps;
    float reflection_strength = 1.f;
    void insert(const ColorStyle& other);
};

}
