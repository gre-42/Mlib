#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <memory>
#include <vector>

namespace Mlib {

struct Bone {
    size_t index;
    // The initial transformation is the transformation in the MHX2-file,
    // the bone transformation is from the BVH-file.
    OffsetAndQuaternion<float, float> initial_absolute_transformation = uninitialized;
    std::vector<std::unique_ptr<Bone>> children;
    UUVector<OffsetAndQuaternion<float, float>> rebase_to_initial_absolute_transform(
        const UUVector<OffsetAndQuaternion<float, float>>& transformations);

    template <class Archive>
    void serialize(Archive& archive) {
        archive(index);
        archive(initial_absolute_transformation);
        archive(children);
    }
private:
    void rebase_to_initial_absolute_transform(
        const UUVector<OffsetAndQuaternion<float, float>>& transformations,
        const OffsetAndQuaternion<float, float>& parent_transformation,
        UUVector<OffsetAndQuaternion<float, float>>& result);
};

}
