#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class StaticBillboardIds {
    StaticBillboardIds(const StaticBillboardIds&) = delete;
    StaticBillboardIds& operator = (const StaticBillboardIds&) = delete;
public:
    StaticBillboardIds(
        const std::vector<TransformationAndBillboardId>& instances,
        uint32_t num_billboard_atlas_components);
    ~StaticBillboardIds();
    void bind(GLuint attribute_index) const;
private:
    const std::vector<TransformationAndBillboardId>& instances_;
    uint32_t num_billboard_atlas_components_;
    mutable GLuint buffer_;
};

}