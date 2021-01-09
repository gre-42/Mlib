#include "Animated_Colored_Vertex_Arrays.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

std::vector<OffsetAndQuaternion<float>> Bone::absolutify(
    const std::vector<OffsetAndQuaternion<float>>& transformations)
{
    std::vector<OffsetAndQuaternion<float>> result;
    result.resize(transformations.size());
#ifndef NDEBUG
    for (OffsetAndQuaternion<float>& r : result) {
        r.offset() = NAN;
    }
#endif
    absolutify(
        transformations,
        OffsetAndQuaternion<float>::identity(),
        result);
#ifndef NDEBUG
    for (const OffsetAndQuaternion<float>& r : result) {
        if (any(isnan(r.offset()))) {
            throw std::runtime_error("Bone transformation contains NAN values");
        }
    }
#endif
    return result;
}

void Bone::absolutify(
    const std::vector<OffsetAndQuaternion<float>>& transformations,
    const OffsetAndQuaternion<float>& parent_transformation,
    std::vector<OffsetAndQuaternion<float>>& result)
{
#ifndef NDEBUG
    if (index >= result.size()) {
        throw std::runtime_error("Bone index too large for result array");
    }
    if (index >= transformations.size()) {
        throw std::runtime_error("Bone index too large for transformations");
    }
#endif
    const OffsetAndQuaternion<float>& m = initial_absolute_transformation;
    OffsetAndQuaternion<float> n = parent_transformation * transformations[index];
    result[index] = n * m.inverse();
    for (const auto& c : children) {
        c->absolutify(
            transformations,
            n,
            result);
    }
}
