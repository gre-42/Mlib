#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>

namespace Mlib {

struct TransformationAndBillboardId;

class DynamicBillboardIds: public DynamicBase<BillboardId> {
    DynamicBillboardIds(const DynamicBillboardIds&) = delete;
    DynamicBillboardIds& operator = (const DynamicBillboardIds&) = delete;
public:
    DynamicBillboardIds(
        size_t max_num_instances,
        BillboardId num_billboard_atlas_components);
    ~DynamicBillboardIds();
    void append(const TransformationAndBillboardId& m);
    void bind(GLuint attribute_index) const;
private:
    BillboardId num_billboard_atlas_components_;
};

}
