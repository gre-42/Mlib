#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <string>

namespace Mlib {

struct ObjMaterial {
    std::string color_texture;
    std::string specular_texture;
    std::string bump_texture;
    bool has_alpha_texture = false;
    FixedArray<float, 3> ambience = fixed_ones<float, 3>();     // One per definition
    FixedArray<float, 3> diffusivity = fixed_ones<float, 3>();  // One per definition
    FixedArray<float, 3> specularity = fixed_ones<float, 3>();  // One per definition
};

}
