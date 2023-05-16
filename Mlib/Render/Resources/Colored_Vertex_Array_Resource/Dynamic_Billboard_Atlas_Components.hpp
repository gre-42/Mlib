#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class DynamicBillboardAtlasComponents {
public:
    DynamicBillboardAtlasComponents(
        uint32_t max_num_instances,
        uint32_t num_billboard_atlas_components);
    ~DynamicBillboardAtlasComponents();
    void append(const TransformationAndBillboardId& m);
    void remove(uint32_t index);
    void bind(GLuint attribute_index) const;
private:
    uint32_t* instances_;
    uint32_t num_billboard_atlas_components_;
    uint32_t max_num_instances_;
    uint32_t num_instances_;
    mutable GLuint buffer_;
};

}
