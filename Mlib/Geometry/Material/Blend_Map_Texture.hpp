#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <cmath>
#include <cstdint>

namespace Mlib {

enum class BlendMapRole {
    NONE = 0,
    SUMMAND = 1,
    DETAIL_BASE = 2,
    DETAIL_MASK_R = 3 | (1 << 6),
    DETAIL_MASK_G = 4 | (1 << 6),
    DETAIL_MASK_B = 5 | (1 << 6),
    DETAIL_MASK_A = 6 | (1 << 6),
    DETAIL_COLOR = 7,
    ANY_DETAIL_MASK = (1 << 6)
};

inline bool any(BlendMapRole a) {
    return a != BlendMapRole::NONE;
}

inline BlendMapRole operator & (BlendMapRole a, BlendMapRole b) {
    return (BlendMapRole)(int(a) & int(b));
}

inline BlendMapRole operator + (BlendMapRole a, uint32_t b) {
    return (BlendMapRole)(uint32_t(a) + b);
}

BlendMapRole blend_map_role_from_string(const std::string& s);

struct BlendMapTexture {
    TextureDescriptor texture_descriptor;
    float min_height = -float(INFINITY);
    float max_height = float(INFINITY);
    OrderableFixedArray<float, 4> distances{ default_linear_distances };
    OrderableFixedArray<float, 3> normal = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 4> cosines{ default_linear_cosines };
    float discreteness = 2.f;
    float scale = 1.f;
    float weight = 1.f;
    BlendMapRole role = BlendMapRole::SUMMAND;
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
        archive(role);
    }
};

}
