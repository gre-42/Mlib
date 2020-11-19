#pragma once
#include <Mlib/Geometry/Plane_Nd.hpp>

namespace Mlib {

/**
 * From: https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Using_homogeneous_coordinates
 */
template <class TData>
FixedArray<TData, 2> intersect_lines(
    const FixedArray<FixedArray<TData, 2>, 2>& l0,
    const FixedArray<FixedArray<TData, 2>, 2>& l1,
    TData width0,
    TData width1,
    bool compute_center = false)
{
    PlaneNd<TData, 2> pl0{l0, compute_center};
    PlaneNd<TData, 2> pl1{l1, compute_center};
    FixedArray<TData, 3> ppl0{pl0.normal(0), pl0.normal(1), pl0.intercept + width0 / 2};
    FixedArray<TData, 3> ppl1{pl1.normal(0), pl1.normal(1), pl1.intercept + width1 / 2};
    auto res = cross(ppl0, ppl1);
    if (std::abs(res(2)) < 1e-7) {
        throw std::runtime_error("Lines do not intersect");
    }
    return FixedArray<TData, 2>{res(0) / res(2), res(1) / res(2)};
}

}
