#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib {

struct BillboardAtlasInstance {
    OrderableFixedArray<float, 2> uv_scale;
    OrderableFixedArray<float, 2> uv_offset;
    OrderableFixedArray<float, 2> vertex_scale;
    std::partial_ordering operator <=> (const BillboardAtlasInstance&) const = default;
};

}
