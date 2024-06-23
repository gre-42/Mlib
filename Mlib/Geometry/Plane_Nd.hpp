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
    PlaneNd(Uninitialized)
        : normal{ uninitialized }
    {}
    PlaneNd(const FixedArray<TData, tndim>& normal, const TData& intercept)
        : normal{ normal }
        , intercept{ intercept }
    {}
    PlaneNd(const FixedArray<TData, tndim>& normal, const FixedArray<TData, tndim>& point_on_plane)
        : PlaneNd{ normal, -dot0d(normal, point_on_plane) }
    {}
    explicit PlaneNd(const FixedArray<FixedArray<TData, 2>, 2>& line, bool compute_center = false)
        : PlaneNd{ line_normal(line), compute_center ? mean(line) : line(0) }
    {}
    explicit PlaneNd(const FixedArray<FixedArray<TData, 3>, 3>& triangle, bool compute_center = false)
        : PlaneNd{ triangle_normal(triangle), compute_center ? mean(triangle) : triangle(0) }
    {}
    template <class TDir>
    PlaneNd transformed(const TransformationMatrix<TDir, TData, 3>& transformation_matrix) const {
        PlaneNd result = uninitialized;
        const auto& n0 = normal;
        const auto& i0 = intercept;
        auto& n1 = result.normal;
        auto& i1 = result.intercept;
        n1 = transformation_matrix.rotate(n0);
        i1 = i0 - dot0d(n1, transformation_matrix.t());
        // i1 = -dot0d(n1, trafo(n0 * (-i0))) = -dot0d(n1, -i0 * n1 + t) = i0 - dot0d(n1, t)
        return result;
    }
    PlaneNd operator - () const {
        return { -normal, -intercept };
    }
    template <class TData2>
    PlaneNd<TData2, tndim> casted() const {
        return { normal.template casted<TData2>(), TData2(intercept) };
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
