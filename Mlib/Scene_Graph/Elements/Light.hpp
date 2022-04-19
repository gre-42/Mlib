#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class ExternalRenderPassType;

struct Light {
    FixedArray<float, 3> ambience{0.5f, 0.5f, 0.5f};
    FixedArray<float, 3> diffusivity{1.f, 1.f, 1.f};
    FixedArray<float, 3> specularity{1.f, 1.f, 1.f};
    std::string node_name;
    ExternalRenderPassType shadow_render_pass;
};

}
