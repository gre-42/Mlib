#include "Animated_Colored_Vertex_Arrays.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

std::vector<FixedArray<float, 4, 4>> Bone::absolutify(
    const std::vector<FixedArray<float, 4, 4>>& transformations)
{
    std::vector<FixedArray<float, 4, 4>> result;
    result.resize(transformations.size());
    for (FixedArray<float, 4, 4>& r : result) {
        r = NAN;
    }
    absolutify(
        transformations,
        fixed_identity_array<float, 4>(),
        fixed_identity_array<float, 4>(),
        result);
    for (const FixedArray<float, 4, 4>& r : result) {
        if (any(isnan(r))) {
            throw std::runtime_error("Bone transformation contains NAN values");
        }
    }
    return result;
}

void Bone::absolutify(
    const std::vector<FixedArray<float, 4, 4>>& transformations,
    const FixedArray<float, 4, 4>& initial_parent_transformation,
    const FixedArray<float, 4, 4>& parent_transformation,
    std::vector<FixedArray<float, 4, 4>>& result)
{
    if (index >= result.size()) {
        throw std::runtime_error("Bone index too large for result array");
    }
    if (index >= transformations.size()) {
        throw std::runtime_error("Bone index too large for transformations");
    }
    FixedArray<float, 4, 4> m = dot2d(initial_parent_transformation, initial_transformation);
    FixedArray<float, 4, 4> n = dot2d(parent_transformation, transformations[index]);
    result[index] = dot2d(n, inverted_scaled_se3(m));
    for (const auto& c : children) {
        c->absolutify(
            transformations,
            m,
            n,
            result);
    }
}
