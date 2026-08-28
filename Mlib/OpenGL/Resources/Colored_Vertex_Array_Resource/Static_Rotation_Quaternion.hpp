#pragma once
#include <Mlib/OpenGL/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Scene_Graph/Instances/Sorted_Vertex_Array_Instances.hpp>
#include <optional>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;
template <class TData>
class Quaternion;

class StaticRotationQuaternion {
    StaticRotationQuaternion(const StaticRotationQuaternion&) = delete;
    StaticRotationQuaternion &operator=(const StaticRotationQuaternion&) = delete;

public:
    StaticRotationQuaternion(
        const SortedTransformedInstances& instances,
        size_t capacity);
    ~StaticRotationQuaternion();
    void update(const SortedTransformedInstances& instances);
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index) const;
    BackgroundCopyState state() const;
    size_t size() const;

private:
    void allocate(
        const SortedTransformedInstances& instances,
        size_t capacity);
    void allocate(const SortedTransformedInstances& instances);
    void wait_and_assign(const SortedTransformedInstances& instances);
    size_t capacity_;
    using RotationQuaternion = Quaternion<float>;
    mutable std::optional<BufferForegroundCopy> buffer_;
    std::vector<RotationQuaternion> quaternions_;
};

}
