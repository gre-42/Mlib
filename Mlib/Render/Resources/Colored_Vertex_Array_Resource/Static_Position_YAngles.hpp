#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class StaticPositionYAngles {
    StaticPositionYAngles(const StaticPositionYAngles&) = delete;
    StaticPositionYAngles& operator = (const StaticPositionYAngles&) = delete;
public:
    explicit StaticPositionYAngles(const std::vector<TransformationAndBillboardId>& instances);
    ~StaticPositionYAngles();
    void bind(GLuint attribute_index) const;
private:
    void deallocate();
    const std::vector<TransformationAndBillboardId>& instances_;
    mutable GLuint buffer_;
    DeallocationToken deallocation_token_;
};

}
