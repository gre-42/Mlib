#pragma once
#include <cmath>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib {

struct BlendMapTexture {
    TextureDescriptor texture_descriptor;
    float min_height = -INFINITY;
    float max_height = INFINITY;
    OrderableFixedArray<float, 4> distances = { 0.f, 0.f, INFINITY, INFINITY };
    OrderableFixedArray<float, 3> normal = { 0.f, 0.f, 0.f };
    float scale = 1;
    float weight = 1;
    std::partial_ordering operator <=> (const BlendMapTexture&) const = default;
};

}
