#pragma once
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Misc/Object.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <cstdint>
#include <list>

namespace Mlib {

struct Bone: public virtual Object {
    Bone();
    Bone(
        uint32_t index,
        const OffsetAndQuaternion<float, float>& initial_absolute_transformation,
        std::list<Bone> children);
    ~Bone();
    uint32_t index;
    // The initial transformation is the transformation in the MHX2-file,
    // the bone transformation is from the BVH-file.
    OffsetAndQuaternion<float, float> initial_absolute_transformation;
    std::list<Bone> children;
    UUVector<OffsetAndQuaternion<float, float>> rebase_to_initial_absolute_transform(
        const UUVector<OffsetAndQuaternion<float, float>>& transformations);

    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
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
