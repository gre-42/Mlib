#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class BlendMode;
enum class WrapMode;

struct BarrierStyle {
    std::string texture;
    FixedArray<float, 2> uv;
    BlendMode blend_mode;
    WrapMode wrap_mode_t;
    bool reorient_uv0;
    float ambience;
    float diffusivity;
    float specularity;
};

}
