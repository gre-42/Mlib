#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <string>

namespace Mlib {

struct ObjMaterial {
    Utf8Path diffuse_texture;
    Utf8Path diffuse_chrominance_texture;
    Utf8Path specular_texture;
    Utf8Path bump_texture;
    float alpha = 1.f;
    bool has_alpha_texture = false;
    FixedArray<float, 3> emissive = fixed_zeros<float, 3>();    // Defaults to zero because it is non-standard and therefore absent in most files
    FixedArray<float, 3> ambient = fixed_ones<float, 3>();      // One per definition
    FixedArray<float, 3> diffuse = fixed_ones<float, 3>();      // One per definition
    FixedArray<float, 3> specular = fixed_ones<float, 3>();     // One per definition
    float specular_exponent = 1.f;
};

}
