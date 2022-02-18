#pragma once
#include <Mlib/Math/Quaternion.hpp>
#include <memory>
#include <vector>

namespace Mlib {

struct Bone {
    size_t index;
    OffsetAndQuaternion<float> initial_absolute_transformation;
    std::vector<std::unique_ptr<Bone>> children;
    std::vector<OffsetAndQuaternion<float>> rebase_to_initial_absolute_transform(
        const std::vector<OffsetAndQuaternion<float>>& transformations);
private:
    void rebase_to_initial_absolute_transform(
        const std::vector<OffsetAndQuaternion<float>>& transformations,
        const OffsetAndQuaternion<float>& parent_transformation,
        std::vector<OffsetAndQuaternion<float>>& result);
};

}
