#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>

namespace Mlib {

struct ShadingFactors {
    FixedArray<float, 3> emissive_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> ambient_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> diffuse_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> specular_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 3> fresnel_ambient_factor = FixedArray<float, 3>(1.f);
    FixedArray<float, 2> fog_distances = default_step_distances;
    FixedArray<float, 3> fog_ambient = FixedArray<float, 3>(0.f);
};

}