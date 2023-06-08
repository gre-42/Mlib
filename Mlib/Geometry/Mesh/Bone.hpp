#pragma once
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <memory>
#include <vector>

namespace Mlib {

struct Bone {
    size_t index;
    OffsetAndQuaternion<float, float> initial_absolute_transformation;
    std::vector<std::unique_ptr<Bone>> children;
    std::vector<OffsetAndQuaternion<float, float>> rebase_to_initial_absolute_transform(
        const std::vector<OffsetAndQuaternion<float, float>>& transformations);

    template <class Archive>
    void serialize(Archive& archive) {
        archive(index);
        archive(initial_absolute_transformation);
        archive(children);
    }
private:
    void rebase_to_initial_absolute_transform(
        const std::vector<OffsetAndQuaternion<float, float>>& transformations,
        const OffsetAndQuaternion<float, float>& parent_transformation,
        std::vector<OffsetAndQuaternion<float, float>>& result);
};

}
