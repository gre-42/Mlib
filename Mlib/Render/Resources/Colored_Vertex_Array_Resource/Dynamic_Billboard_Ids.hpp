#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>

namespace Mlib {

struct TransformationAndBillboardId;

class DynamicBillboardIds: public DynamicBase<uint32_t> {
    DynamicBillboardIds(const DynamicBillboardIds&) = delete;
    DynamicBillboardIds& operator = (const DynamicBillboardIds&) = delete;
public:
    DynamicBillboardIds(
        size_t max_num_instances,
        uint32_t num_billboard_atlas_components);
    ~DynamicBillboardIds();
    void append(const TransformationAndBillboardId& m);
    void bind(GLuint attribute_index) const;
private:
    uint32_t num_billboard_atlas_components_;
};

}
