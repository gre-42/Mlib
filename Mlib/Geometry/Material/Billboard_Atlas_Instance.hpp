#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>

namespace Mlib {

enum class ExternalRenderPassType;

struct BillboardAtlasInstance {
    OrderableFixedArray<float, 2> uv_scale = uninitialized;
    OrderableFixedArray<float, 2> uv_offset = uninitialized;
    uint8_t texture_layer;
    OrderableFixedArray<float, 3> vertex_scale = uninitialized;
    ScenePos max_center_distance2;
    ExternalRenderPassType occluder_pass;
    OrderableFixedArray<float, 4> alpha_distances = { default_linear_distances };
    std::partial_ordering operator <=> (const BillboardAtlasInstance&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(uv_scale);
        archive(uv_offset);
        archive(texture_layer);
        archive(vertex_scale);
        archive(max_center_distance2);
        archive(occluder_pass);
        archive(alpha_distances);
    }
};

}
