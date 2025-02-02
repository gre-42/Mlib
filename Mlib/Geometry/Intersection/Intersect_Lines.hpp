#pragma once
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

// From: https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Using_homogeneous_coordinates
template <class TPos>
bool intersect_planes(
    FixedArray<TPos, 2>& intersection,
    const PlaneNd<TPos, TPos, 2>& pl0,
    const PlaneNd<TPos, TPos, 2>& pl1,
    const TPos& width0,
    const TPos& width1)
{
    FixedArray<TPos, 2, 3> ppl{
        FixedArray<TPos, 3>{pl0.normal(0), pl0.normal(1), pl0.intercept + width0 / 2},
        FixedArray<TPos, 3>{pl1.normal(0), pl1.normal(1), pl1.intercept + width1 / 2}};
    auto res = cross(ppl[0], ppl[1]);
    if (std::abs(res(2)) < 1e-7) {
        return false;
    }
    intersection = FixedArray<TPos, 2>{
        res(0) / res(2),
        res(1) / res(2) };
    return true;
}

template <class TData>
bool intersect_rays(
    FixedArray<TData, 2>& intersection,
    const FixedArray<TData, 2>& p0,
    const FixedArray<TData, 2>& v0,
    const FixedArray<TData, 2>& p1,
    const FixedArray<TData, 2>& v1,
    const TData& width0,
    const TData& width1)
{
    FixedArray<TData, 2> n0{ v0(1), -v0(0) };
    FixedArray<TData, 2> n1{ v1(1), -v1(0) };
    FixedArray<PlaneNd<TData, TData, 2>, 2> pl{
        PlaneNd<TData, TData, 2>{ n0, p0 },
        PlaneNd<TData, TData, 2>{ n1, p1 }};
    return intersect_planes(intersection, pl(0), pl(1), width0, width1);
}

/**
 * From: https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Using_homogeneous_coordinates
 */
template <class TData>
bool intersect_lines(
    FixedArray<TData, 2>& intersection,
    const FixedArray<TData, 2, 2>& l0,
    const FixedArray<TData, 2, 2>& l1,
    const TData& width0,
    const TData& width1,
    bool compute_center = false,
    bool check_bounds = false)
{
    if (check_bounds) {
        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 2; ++j) {
                if (all(l0[i] == l1[j])) {
                    return false;
                }
            }
        }
    }
    FixedArray<PlaneNd<TData, TData, 2>, 2> pl{
        PlaneNd<TData, TData, 2>{l0, compute_center},
        PlaneNd<TData, TData, 2>{l1, compute_center}};
    if (!intersect_planes(intersection, pl(0), pl(1), width0, width1)) {
        return false;
    }
    if (check_bounds) {
        // Maybe use "transform_to_line_coordinates" here?
        FixedArray<TData, 2, 2, 2> ls{l0, l1};
        for (size_t i = 0; i < 2; ++i) {
            FixedArray<TData, 2> v{ls[i][1] - ls[i][0]};
            TData x = dot0d(v, intersection);
            TData a = dot0d(v, ls[i][0]);
            TData b = dot0d(v, ls[i][1]);
            if (x < a || x > b) {
                return false;
            }
        }
    }
    return true;
}

template <class TData>
FixedArray<TData, 2> intersect_lines(
    const FixedArray<TData, 2, 2>& l0,
    const FixedArray<TData, 2, 2>& l1,
    const TData& width0,
    const TData& width1,
    bool compute_center = false)
{
    FixedArray<TData, 2> intersection = uninitialized;
    if (!intersect_lines(intersection, l0, l1, width0, width1, compute_center)) {
        THROW_OR_ABORT("Lines do not intersect");
    }
    return intersection;
}

}
