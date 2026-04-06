#pragma once
#include <compare>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

namespace Mlib {

class IGpuVertexArray;

struct AttributeIndices {
    uint32_t idx_position;
    uint32_t idx_color;
    uint32_t idx_normal;
    uint32_t idx_tangent;
    uint32_t idx_instance_attrs;
    uint32_t idx_rotation_quaternion;
    uint32_t idx_billboard_ids;
    uint32_t idx_bone_indices;
    uint32_t idx_bone_weights;
    uint32_t idx_texture_layer;
    uint32_t idx_interior_mapping_bottom_left;
    uint32_t idx_interior_mapping_uvmap;
    uint32_t idx_uv_0;
    uint32_t idx_uv_1;
    uint32_t uv_count;
    uint32_t idx_cweight_0;
    uint32_t cweight_count;
    uint32_t idx_alpha;
    std::strong_ordering operator <=> (const AttributeIndices&) const = default;
};

struct AttributeIndexCalculator {
    bool has_position;
    bool has_color;
    bool has_normal;
    bool has_tangent;
    bool has_instance_attrs;
    bool has_rotation_quaternion;
    bool has_billboard_ids;
    bool has_bone_indices;
    bool has_bone_weights;
    bool has_texture_layer;
    bool has_interior_mapping_bottom_left;
    bool has_interior_mapping_multiplier;
    size_t nuvs;
    size_t ncweights;
    bool has_alpha;

    AttributeIndices build() const;
    std::partial_ordering operator <=> (const AttributeIndexCalculator&) const = default;
};

AttributeIndexCalculator get_attribute_index_calculator(const IGpuVertexArray& gva);
std::ostream& operator << (std::ostream& ostr, const AttributeIndexCalculator& attr_idc);

}
