#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

enum class ExternalRenderPassType;

struct Light {
    FixedArray<float, 3> ambient{1.f, 1.f, 1.f};
    FixedArray<float, 3> diffuse{1.f, 1.f, 1.f};
    FixedArray<float, 3> specular{1.f, 1.f, 1.f};
    FixedArray<float, 3> fresnel_ambient{1.f, 1.f, 1.f};
    std::string resource_suffix;
    ExternalRenderPassType shadow_render_pass;
};

}
