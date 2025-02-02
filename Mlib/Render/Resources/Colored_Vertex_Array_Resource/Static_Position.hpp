#pragma once
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <cstdint>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;
template <typename TData, size_t... tshape>
class FixedArray;

class StaticPosition {
    StaticPosition(const StaticPosition &) = delete;
    StaticPosition &operator=(const StaticPosition &) = delete;

public:
    explicit StaticPosition(const std::vector<TransformationAndBillboardId> &instances);
    ~StaticPosition();
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index, TaskLocation task_location) const;

private:
    using Position = FixedArray<float, 3>;
    const std::vector<TransformationAndBillboardId> &instances_;
    mutable BufferBackgroundCopy buffer_;
    std::vector<Position> positions_;
};

}
