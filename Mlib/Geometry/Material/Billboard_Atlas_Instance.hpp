#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib {

enum class ExternalRenderPassType;

struct BillboardAtlasInstance {
    OrderableFixedArray<float, 2> uv_scale;
    OrderableFixedArray<float, 2> uv_offset;
    OrderableFixedArray<float, 2> vertex_scale;
    bool is_small;
    ExternalRenderPassType occluder_pass;
    std::partial_ordering operator <=> (const BillboardAtlasInstance&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(uv_scale, uv_offset, vertex_scale, is_small);
    }
};

}
