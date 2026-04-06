#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <compare>
#include <cstdint>
#include <memory>

namespace Mlib {

class IGpuVertexData;

struct SortableDeferredVertexData {
    int32_t layer;
    std::shared_ptr<IGpuVertexData> vertex_array;
    std::strong_ordering operator <=> (const SortableDeferredVertexData&) const = default;
};

}
