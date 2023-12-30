#pragma once
#include <Mlib/Geometry/Line_Normal.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <ostream>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TData, size_t tndim>
class PlaneNd {
public:
    PlaneNd() = default;
    PlaneNd(const FixedArray<TData, tndim>& normal, const FixedArray<TData, tndim>& point_on_plane)
    : normal{normal},
      intercept{-dot0d(normal, point_on_plane)}
    {}
    explicit PlaneNd(const FixedArray<FixedArray<TData, 2>, 2>& line, bool compute_center = false)
    : PlaneNd{line_normal(line), compute_center ? mean(line) : line(0)}
    {}
    explicit PlaneNd(const FixedArray<FixedArray<TData, 3>, 3>& triangle, bool compute_center = false)
    : PlaneNd{triangle_normal(triangle), compute_center ? mean(triangle) : triangle(0)}
    {}
    template <class TDir>
    PlaneNd transformed(const TransformationMatrix<TDir, TData, 3>& transformation_matrix) const {
        PlaneNd result;
        const auto& n0 = normal;
        const auto& i0 = intercept;
        auto& n1 = result.normal;
        auto& i1 = result.intercept;
        n1 = transformation_matrix.rotate(n0 TEMPLATEV casted<TDir>()) TEMPLATEV casted<TData>();
        i1 = i0 - dot0d(n1, transformation_matrix.t());
        // i1 = -dot0d(n1, trafo(n0 * (-i0))) = -dot0d(n1, -i0 * n1 + t) = i0 - dot0d(n1, t)
    }
    FixedArray<TData, tndim> normal;
    TData intercept;
};

template <class TData, size_t tndim>
std::ostream& operator << (std::ostream& ostr, const PlaneNd<TData, tndim>& plane) {
    ostr << "normal: " << plane.normal << " intercept: " << plane.intercept;
    return ostr;
}

}
