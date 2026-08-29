#pragma once
#include <Mlib/OpenGL/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <cstdint>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class StaticPosition {
    StaticPosition(const StaticPosition &) = delete;
    StaticPosition &operator=(const StaticPosition &) = delete;

public:
    StaticPosition(
        const SortedLookatInstances& instances,
        size_t capacity);
    ~StaticPosition();
    void update(const SortedLookatInstances& instances);
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index) const;
    BackgroundCopyState state() const;
    size_t size() const;

private:
    void wait_and_assign(const SortedLookatInstances& instances);
    size_t capacity_;
    using Position = FixedArray<float, 3>;
    mutable BufferForegroundCopy buffer_;
    std::vector<Position> positions_;
};

}
