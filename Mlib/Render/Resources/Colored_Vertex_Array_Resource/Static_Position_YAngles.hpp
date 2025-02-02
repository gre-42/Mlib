#pragma once
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <cstdint>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;
template <typename TData, size_t... tshape>
class FixedArray;

class StaticPositionYAngles {
    StaticPositionYAngles(const StaticPositionYAngles &) = delete;
    StaticPositionYAngles &operator=(const StaticPositionYAngles &) = delete;

public:
    explicit StaticPositionYAngles(const std::vector<TransformationAndBillboardId> &instances);
    ~StaticPositionYAngles();
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index, TaskLocation task_location) const;

private:
    using Position = FixedArray<float, 4>;
    const std::vector<TransformationAndBillboardId> &instances_;
    mutable BufferBackgroundCopy buffer_;
    std::vector<Position> positions_;
};

}
