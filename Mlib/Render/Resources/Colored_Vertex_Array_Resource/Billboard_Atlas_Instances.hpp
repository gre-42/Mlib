#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class BillboardAtlasInstances {
public:
    explicit BillboardAtlasInstances(const std::vector<TransformationAndBillboardId>& instances);
    ~BillboardAtlasInstances();
    void bind(GLuint attribute_index, uint32_t num_billboard_atlas_instances) const;
private:
    const std::vector<TransformationAndBillboardId>& instances_;
    mutable GLuint buffer_;
};

}
