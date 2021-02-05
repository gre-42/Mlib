#pragma once
#include <cmath>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>

namespace Mlib {

struct BlendMapTexture {
    TextureDescriptor texture_descriptor;
    float min_height = -INFINITY;
    float max_height = INFINITY;
    std::partial_ordering operator <=> (const BlendMapTexture&) const = default;
};

}
