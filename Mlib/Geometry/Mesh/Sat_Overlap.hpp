#pragma once
#include <cstddef>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;

// double get_overlap(
//     const CollisionTriangleSphere& t0,
//     const IIntersectableMesh& mesh1);

double sat_overlap_signed(
    const FixedArray<double, 3>& n,
    const std::set<OrderableFixedArray<double, 3>>& vertices0,
    const std::set<OrderableFixedArray<double, 3>>& vertices1);

void sat_overlap_unsigned(
    const FixedArray<double, 3>& l,
    const std::set<OrderableFixedArray<double, 3>>& vertices0,
    const std::set<OrderableFixedArray<double, 3>>& vertices1,
    double& overlap0,
    double& overlap1);

}
