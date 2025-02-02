#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>

namespace Mlib {

struct TransformationAndBillboardId;

class DynamicInstanceContinuousTextureLayer: public DynamicBase<float> {
    DynamicInstanceContinuousTextureLayer(const DynamicInstanceContinuousTextureLayer&) = delete;
    DynamicInstanceContinuousTextureLayer& operator = (const DynamicInstanceContinuousTextureLayer&) = delete;
public:
    explicit DynamicInstanceContinuousTextureLayer(size_t max_num_instances);
    ~DynamicInstanceContinuousTextureLayer();
    void bind(GLuint attribute_index) const;
};

}
