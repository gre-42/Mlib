#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib {

enum class ExternalRenderPassType;

struct BillboardAtlasInstance {
    OrderableFixedArray<float, 2> uv_scale;
    OrderableFixedArray<float, 2> uv_offset;
    OrderableFixedArray<float, 2> vertex_scale;
    double max_center_distance;
    ExternalRenderPassType occluder_pass;
    OrderableFixedArray<float, 4> alpha_distances = { default_linear_distances };
    std::partial_ordering operator <=> (const BillboardAtlasInstance&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(uv_scale);
        archive(uv_offset);
        archive(vertex_scale);
        archive(max_center_distance);
        archive(occluder_pass);
    }
};

}
