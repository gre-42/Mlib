#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct CollisionRidgeSphere;
class IIntersectableMesh;

void get_overlap2(
    const IIntersectableMesh& mesh0,
    const CollisionRidgeSphere& e1,
    double& min_overlap,
    FixedArray<double, 3>& normal);

}
