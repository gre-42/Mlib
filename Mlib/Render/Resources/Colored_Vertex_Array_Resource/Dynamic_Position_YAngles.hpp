#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct TransformationAndBillboardId;

class DynamicPositionYAngles: public DynamicBase<FixedArray<float, 4>> {
    DynamicPositionYAngles(const DynamicPositionYAngles&) = delete;
    DynamicPositionYAngles& operator = (const DynamicPositionYAngles&) = delete;
public:
    explicit DynamicPositionYAngles(size_t max_num_instances);
    ~DynamicPositionYAngles();
    void append(const TransformationAndBillboardId& m);
    void bind(GLuint attribute_index) const;
};

}
