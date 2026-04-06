#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Shading.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <string>

namespace Mlib {

enum class BlendMode;
enum class WrapMode;

struct BarrierStyle {
    FPath texture;
    FixedArray<float, 2> uv;
    float depth;
    FixedArray<float, 3> depth_color;
    BlendMode blend_mode;
    bool cull_faces;
    bool reorient_uv0;
    Shading shading;
};

}
