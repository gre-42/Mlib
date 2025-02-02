#pragma once
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>

namespace Mlib {

struct TransformationAndBillboardId;

class DynamicRotationQuaternion: public DynamicBase<Quaternion<float>> {
    DynamicRotationQuaternion(const DynamicRotationQuaternion&) = delete;
    DynamicRotationQuaternion& operator = (const DynamicRotationQuaternion&) = delete;
public:
    explicit DynamicRotationQuaternion(size_t max_num_instances);
    ~DynamicRotationQuaternion();
    void append(const TransformationAndBillboardId& m);
    void bind(GLuint attribute_index) const;
};

}
