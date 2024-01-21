#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <string>

namespace Mlib {

struct ObjMaterial {
    std::string color_texture;
    std::string specular_texture;
    std::string bump_texture;
    float alpha = 1.f;
    bool has_alpha_texture = false;
    FixedArray<float, 3> emissive = fixed_zeros<float, 3>();    // Defaults to zero because it is non-standard and therefore absent in most files
    FixedArray<float, 3> ambient = fixed_ones<float, 3>();      // One per definition
    FixedArray<float, 3> diffuse = fixed_ones<float, 3>();      // One per definition
    FixedArray<float, 3> specular = fixed_ones<float, 3>();     // One per definition
    float specular_exponent = 1.f;
};

}
