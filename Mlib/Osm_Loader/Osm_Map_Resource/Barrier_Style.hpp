#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Shading.hpp>
#include <string>

namespace Mlib {

enum class BlendMode;
enum class WrapMode;

struct BarrierStyle {
    std::string texture;
    FixedArray<float, 2> uv;
    BlendMode blend_mode;
    bool reorient_uv0;
    Shading shading;
};

}
