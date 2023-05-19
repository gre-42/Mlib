#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct TransformationAndBillboardId;

class DynamicPosition: public DynamicBase<FixedArray<float, 3>> {
    DynamicPosition(const DynamicPosition&) = delete;
    DynamicPosition& operator = (const DynamicPosition&) = delete;
public:
    explicit DynamicPosition(size_t max_num_instances);
    ~DynamicPosition();
    void append(const TransformationAndBillboardId& m);
    void bind(GLuint attribute_index) const;
};

}
