#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class StaticPosition {
    StaticPosition(const StaticPosition&) = delete;
    StaticPosition& operator = (const StaticPosition&) = delete;
public:
    explicit StaticPosition(const std::vector<TransformationAndBillboardId>& instances);
    ~StaticPosition();
    void bind(GLuint attribute_index) const;
private:
    const std::vector<TransformationAndBillboardId>& instances_;
    mutable GLuint buffer_;
};

}
