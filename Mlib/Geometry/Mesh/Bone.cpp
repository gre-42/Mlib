#include "Bone.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

UUVector<OffsetAndQuaternion<float, float>> Bone::rebase_to_initial_absolute_transform(
    const UUVector<OffsetAndQuaternion<float, float>>& transformations)
{
    UUVector<OffsetAndQuaternion<float, float>> result;
    result.resize(transformations.size());
#ifndef NDEBUG
    for (OffsetAndQuaternion<float, float>& r : result) {
        r.t = NAN;
    }
#endif
    rebase_to_initial_absolute_transform(
        transformations,
        OffsetAndQuaternion<float, float>::identity(),
        result);
#ifndef NDEBUG
    for (const OffsetAndQuaternion<float, float>& r : result) {
        if (any(Mlib::isnan(r.t))) {
            THROW_OR_ABORT("Bone transformation contains NAN values");
        }
    }
#endif
    return result;
}

void Bone::rebase_to_initial_absolute_transform(
    const UUVector<OffsetAndQuaternion<float, float>>& transformations,
    const OffsetAndQuaternion<float, float>& parent_transformation,
    UUVector<OffsetAndQuaternion<float, float>>& result)
{
#ifndef NDEBUG
    if (index >= result.size()) {
        THROW_OR_ABORT("Bone index too large for result array");
    }
    if (index >= transformations.size()) {
        THROW_OR_ABORT("Bone index too large for transformations");
    }
#endif
    const OffsetAndQuaternion<float, float>& m = initial_absolute_transformation;
    OffsetAndQuaternion<float, float> n = parent_transformation * transformations[index];
    result[index] = n * m.inverse();
    for (const auto& c : children) {
        c->rebase_to_initial_absolute_transform(
            transformations,
            n,
            result);
    }
}
