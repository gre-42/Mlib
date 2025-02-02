#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace Mlib {

enum class BlendMapRole {
    ANY_DETAIL_MASK = (1 << 6),
    
    NONE                    = 0,
    SUMMAND                 = 1,
    DETAIL_BASE             = 2,
    DETAIL_MASK_R           = 3 | ANY_DETAIL_MASK,
    DETAIL_MASK_G           = 4 | ANY_DETAIL_MASK,
    DETAIL_MASK_B           = 5 | ANY_DETAIL_MASK,
    DETAIL_MASK_A           = 6 | ANY_DETAIL_MASK,
    DETAIL_COLOR            = 7
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

BlendMapRole blend_map_role_from_string(std::string_view s);

enum class BlendMapUvSource: uint32_t {
    VERTICAL0,
    VERTICAL1,
    VERTICAL2,
    VERTICAL3,
    VERTICAL4,
    VERTICAL_LAST = VERTICAL4,
    HORIZONTAL
};

inline uint32_t operator - (BlendMapUvSource a, BlendMapUvSource b) {
    return (uint32_t)a - (uint32_t)b;
}

BlendMapUvSource blend_map_uv_source_from_string(std::string_view s);

enum class BlendMapReductionOperation {
    PLUS,
    MINUS,
    TIMES,
    FEATHER,
    INVERT,
    BLEND,
    REPLACE_COLOR,
    COLORIZE
};

BlendMapReductionOperation blend_map_reduction_operation_from_string(std::string_view s);

enum class BlendMapReweightMode {
    UNDEFINED,
    ENABLED,
    DISABLED
};

BlendMapReweightMode blend_map_reweight_mode_from_string(std::string_view s);

struct BlendMapTexture {
    TextureDescriptor texture_descriptor;
    float min_height = -float(INFINITY);
    float max_height = float(INFINITY);
    OrderableFixedArray<float, 4> distances{ default_linear_distances };
    OrderableFixedArray<float, 3> normal = { 0.f, 0.f, 0.f };
    OrderableFixedArray<float, 4> cosines{ default_linear_cosines };
    float discreteness = 2.f;
    OrderableFixedArray<float, 2> offset = { 0.f, 0.f };
    OrderableFixedArray<float, 2> scale = { 1.f, 1.f };
    float weight = 1.f;
    uint32_t cweight_id = UINT32_MAX;
    float plus = 0.f;
    float min_detail_weight = 0.f;
    BlendMapRole role = BlendMapRole::SUMMAND;
    BlendMapUvSource uv_source = BlendMapUvSource::VERTICAL0;
    BlendMapReductionOperation reduction = BlendMapReductionOperation::PLUS;
    BlendMapReweightMode reweight_mode = BlendMapReweightMode::UNDEFINED;
    // Don't forget to update "modifiers_hash" and "serialize" when adding parameters.
    size_t modifiers_hash() const;
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
        archive(offset);
        archive(scale);
        archive(weight);
        archive(cweight_id);
        archive(plus);
        archive(min_detail_weight);
        archive(role);
        archive(uv_source);
        archive(reduction);
        archive(reweight_mode);
    }
};

}
