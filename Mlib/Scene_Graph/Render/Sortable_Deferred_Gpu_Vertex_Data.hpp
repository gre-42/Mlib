#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <compare>
#include <cstdint>
#include <memory>

namespace Mlib {

enum class BlendMode;
enum class DepthFunc;
class IGpuVertexData;

struct SortableDeferredVertexData {
    BlendMode blend_mode;
    int32_t continuous_blending_z_order;
    DepthFunc depth_func;
    std::shared_ptr<IGpuVertexData> vertex_array;
    std::strong_ordering operator <=> (const SortableDeferredVertexData&) const = default;
};

}
