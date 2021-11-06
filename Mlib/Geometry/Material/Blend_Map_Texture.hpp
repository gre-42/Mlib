#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <cmath>

namespace Mlib {

struct BlendMapTexture {
    TextureDescriptor texture_descriptor;
    float min_height = -float(INFINITY);
    float max_height = float(INFINITY);
    OrderableFixedArray<float, 4> distances{ default_distances };
    OrderableFixedArray<float, 3> normal = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 4> cosines{ default_cosines };
    float discreteness = 2;
    float scale = 1;
    float weight = 1;
    std::partial_ordering operator <=> (const BlendMapTexture&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(texture_descriptor);
        archive(min_height);
        archive(max_height);
        archive(distances);
        archive(normal);
        archive(cosines);
        archive(discreteness);
        archive(scale);
        archive(weight);
    }
};

}
