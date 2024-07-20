#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct TransformationAndBillboardId;

class DynamicRotationAxis: public DynamicBase<FixedArray<float, 3>> {
    DynamicRotationAxis(const DynamicRotationAxis&) = delete;
    DynamicRotationAxis& operator = (const DynamicRotationAxis&) = delete;
public:
    DynamicRotationAxis(size_t max_num_instances, size_t axis);
    ~DynamicRotationAxis();
    void append(const TransformationAndBillboardId& m);
    void bind(GLuint attribute_index) const;
private:
    size_t axis_;
};

}
