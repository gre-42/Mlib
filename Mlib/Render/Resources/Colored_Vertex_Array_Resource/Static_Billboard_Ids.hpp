#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
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
    void deallocate();
    const std::vector<TransformationAndBillboardId>& instances_;
    uint32_t num_billboard_atlas_components_;
    mutable GLuint buffer_;
    DeallocationToken deallocation_token_;
};

}
