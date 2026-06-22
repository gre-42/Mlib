#include "Bone.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <stdexcept>

using namespace Mlib;

Bone::Bone()
    : initial_absolute_transformation{uninitialized}
{}

Bone::Bone(
    uint32_t index,
    const OffsetAndQuaternion<float, float>& initial_absolute_transformation,
    std::list<Bone> children)
    : index{index}
    , initial_absolute_transformation{initial_absolute_transformation}
    , children(std::move(children))
{}

Bone::~Bone() = default;

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
            throw std::runtime_error("Bone transformation contains NAN values");
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
        throw std::runtime_error("Bone index too large for result array");
    }
    if (index >= transformations.size()) {
        throw std::runtime_error("Bone index too large for transformations");
    }
#endif
    const OffsetAndQuaternion<float, float>& m = initial_absolute_transformation;
    OffsetAndQuaternion<float, float> n = parent_transformation * transformations[index];
    result[index] = n * m.inverse();
    for (auto& c : children) {
        c.rebase_to_initial_absolute_transform(
            transformations,
            n,
            result);
    }
}
