#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/String.hpp>

namespace Mlib {

enum class BlendMode {
    OFF,
    BINARY,
    CONTINUOUS
};

enum class ClampMode {
    EDGE,
    REPEAT
};

enum class AggregateMode {
    OFF,
    ONCE,
    SORTED_CONTINUOUSLY,
    INSTANCES_ONCE,
    INSTANCES_SORTED_CONTINUOUSLY
};

enum class OccludedType {
    OFF,
    LIGHT_MAP_COLOR,
    LIGHT_MAP_DEPTH
};

enum class OccluderType {
    OFF,
    WHITE,
    BLACK
};

inline BlendMode blend_mode_from_string(const std::string& str) {
    if (str == "off") {
        return BlendMode::OFF;
    } else if (str == "binary") {
        return BlendMode::BINARY;
    } else if (str == "continuous") {
        return BlendMode::CONTINUOUS;
    }
    throw std::runtime_error("Unknown blend mode");
}

inline AggregateMode aggregate_mode_from_string(const std::string& str) {
    if (str == "off") {
        return AggregateMode::OFF;
    } else if (str == "once") {
        return AggregateMode::ONCE;
    } else if (str == "sorted") {
        return AggregateMode::SORTED_CONTINUOUSLY;
    } if (str == "instances_once") {
        return AggregateMode::INSTANCES_ONCE;
    } if (str == "instances_sorted") {
        return AggregateMode::INSTANCES_SORTED_CONTINUOUSLY;
    }
    throw std::runtime_error("Unknown aggregate mode");
}

inline OccludedType occluded_type_from_string(const std::string& str) {
    if (str == "off") {
        return OccludedType::OFF;
    } else if (str == "color") {
        return OccludedType::LIGHT_MAP_COLOR;
    } else if (str == "depth") {
        return OccludedType::LIGHT_MAP_DEPTH;
    }
    throw std::runtime_error("Unknown occluded type");
}

inline OccluderType occluder_type_from_string(const std::string& str) {
    if (str == "off") {
        return OccluderType::OFF;
    } else if (str == "white") {
        return OccluderType::WHITE;
    } else if (str == "black") {
        return OccluderType::BLACK;
    }
    throw std::runtime_error("Unknown occluder type");
}

struct Material {
    std::string texture = "";
    OccludedType occluded_type = OccludedType::OFF;
    OccluderType occluder_type = OccluderType::BLACK;
    bool occluded_by_black = true;
    std::string mixed_texture;
    size_t overlap_npixels = 5;
    std::string dirt_texture;
    BlendMode blend_mode = BlendMode::OFF;
    ClampMode clamp_mode_s = ClampMode::REPEAT;
    ClampMode clamp_mode_t = ClampMode::REPEAT;
    bool collide = true;
    AggregateMode aggregate_mode = AggregateMode::OFF;
    bool is_small = false;
    bool cull_faces = true;
    OrderableFixedArray<float, 3> ambience{0.5f, 0.5f, 0.5f};
    OrderableFixedArray<float, 3> diffusivity{1.f, 1.f, 1.f};
    OrderableFixedArray<float, 3> specularity{1.f, 1.f, 1.f};
    std::strong_ordering operator <=> (const Material&) const = default;
};

}
