#pragma once
#include <Mlib/OpenGL/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
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
    explicit StaticPositionYAngles(
        const SortedYAngleInstances& instances,
        size_t capacity);
    ~StaticPositionYAngles();
    void update(const SortedYAngleInstances& instances);
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index) const;
    BackgroundCopyState state() const;
    size_t size() const;

private:
    void wait_and_assign(const SortedYAngleInstances& instances);
    size_t capacity_;
    using Position = FixedArray<float, 4>;
    mutable BufferForegroundCopy buffer_;
    std::vector<Position> positions_;
};

}
