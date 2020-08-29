#pragma once
#include <Mlib/Geometry/Line_Normal.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Mean.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class PlaneNd {
public:
    PlaneNd() = default;
    PlaneNd(const FixedArray<TData, tndim>& normal, const FixedArray<TData, tndim>& point_on_plane)
    : normal_{normal},
      intercept_{-dot0d(normal, point_on_plane)}
    {}
    explicit PlaneNd(const FixedArray<FixedArray<TData, 2>, 2>& line, bool compute_center = false)
    : PlaneNd{line_normal(line), compute_center ? mean(line) : line(0)}
    {}
    explicit PlaneNd(const FixedArray<FixedArray<TData, 3>, 3>& triangle, bool compute_center = false)
    : PlaneNd{triangle_normal(triangle), compute_center ? mean(triangle) : triangle(0)}
    {}
    FixedArray<TData, tndim> normal_;
    TData intercept_;
};

}
