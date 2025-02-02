#pragma once
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;
template <class TData>
class Quaternion;

class StaticRotationQuaternion {
    StaticRotationQuaternion(const StaticRotationQuaternion&) = delete;
    StaticRotationQuaternion &operator=(const StaticRotationQuaternion&) = delete;

public:
    explicit StaticRotationQuaternion(
        const std::vector<TransformationAndBillboardId>& instances);
    ~StaticRotationQuaternion();
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index, TaskLocation task_location) const;

private:
    using RotationQuaternion = Quaternion<float>;
    const std::vector<TransformationAndBillboardId>& instances_;
    mutable BufferBackgroundCopy buffer_;
    std::vector<RotationQuaternion> quaternions_;
};

}
