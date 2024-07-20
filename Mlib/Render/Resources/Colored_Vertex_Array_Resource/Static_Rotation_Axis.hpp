#pragma once
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <cstdint>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;
template <typename TData, size_t... tshape>
class FixedArray;

class StaticRotationAxis {
    StaticRotationAxis(const StaticRotationAxis&) = delete;
    StaticRotationAxis &operator=(const StaticRotationAxis&) = delete;

public:
    explicit StaticRotationAxis(
        const std::vector<TransformationAndBillboardId>& instances,
        size_t axis);
    ~StaticRotationAxis();
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index) const;

private:
    using RotationAxis = FixedArray<float, 3>;
    const std::vector<TransformationAndBillboardId>& instances_;
    mutable BufferBackgroundCopy buffer_;
    std::vector<RotationAxis> axes_;
};

}
