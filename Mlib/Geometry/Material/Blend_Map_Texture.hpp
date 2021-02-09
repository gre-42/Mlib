#pragma once
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <cmath>

namespace Mlib {

static const OrderableFixedArray<float, 4> default_distances{ 0.f, 0.f, float(INFINITY), float(INFINITY) };
static const OrderableFixedArray<float, 4> default_cosines{ -1.f, -1.f, 1.f, 1.f };

struct BlendMapTexture {
    TextureDescriptor texture_descriptor;
    float min_height = -float(INFINITY);
    float max_height = float(INFINITY);
    OrderableFixedArray<float, 4> distances{ default_distances };
    OrderableFixedArray<float, 3> normal = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 4> cosines{ default_cosines };
    float scale = 1;
    float weight = 1;
    std::partial_ordering operator <=> (const BlendMapTexture&) const = default;
};

}
