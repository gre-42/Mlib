#include "Animated_Colored_Vertex_Arrays.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

std::vector<FixedArray<float, 4, 4>> Bone::absolutify(
    const std::vector<FixedArray<float, 4, 4>>& transformations)
{
    std::vector<FixedArray<float, 4, 4>> result;
    result.resize(transformations.size());
    absolutify(transformations, fixed_identity_array<float, 4>(), result);
    return result;
}

void Bone::absolutify(
    const std::vector<FixedArray<float, 4, 4>>& transformations,
    const FixedArray<float, 4, 4>& parent_transformation,
    std::vector<FixedArray<float, 4, 4>>& result)
{
    if (index >= result.size()) {
        throw std::runtime_error("Bone index too large for result array");
    }
    result[index] = dot2d(parent_transformation, transformation);
    for (const auto& c : children) {
        c->absolutify(transformations, result[index], result);
    }
}
    