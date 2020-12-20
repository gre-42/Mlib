#pragma once
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <map>

namespace Mlib {

struct ObjMaterial {
    std::string color_texture = "";
    std::string bump_texture = "";
    bool has_alpha_texture = false;
    FixedArray<float, 3> ambience = fixed_ones<float, 3>();
    FixedArray<float, 3> diffusivity = fixed_ones<float, 3>();
    FixedArray<float, 3> specularity = fixed_ones<float, 3>();
};

std::map<std::string, ObjMaterial> load_mtllib(const std::string& filename, bool werror);

}
