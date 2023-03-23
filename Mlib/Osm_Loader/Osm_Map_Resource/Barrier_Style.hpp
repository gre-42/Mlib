#pragma once
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
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
