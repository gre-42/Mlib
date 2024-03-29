#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Fresnel.hpp>

namespace Mlib {

struct DynamicStyle {
    FixedArray<float, 3> emissive{0.f, 0.f, 0.f};
};

}
