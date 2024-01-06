#pragma once
#include <Mlib/Geometry/Plane_Nd.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TData, size_t tndim>
class BoundingSphere;

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TData, size_t tnvertices>
class ConvexPolygon3D {
public:
    ConvexPolygon3D() {}
    ConvexPolygon3D(
        const FixedArray<PlaneNd<TData, 3>, tnvertices>& edges,
        const PlaneNd<TData, 3>& plane)
        : edges_{ edges }
        , plane_{ plane }
    {}
    ConvexPolygon3D(const FixedArray<FixedArray<TData, 3>, tnvertices>& corners)
    {
        static_assert(tnvertices >= 3);
        plane_ = PlaneNd<TData, 3>{ FixedArray<FixedArray<TData, 3>, 3>{
            corners(0),
            corners(1),
            corners(2) } };
        for (size_t i = 0; i < tnvertices; ++i) {
            auto dir = corners((i + 1) % tnvertices) - corners(i);
            auto edge_normal = cross(plane_.normal, dir);
            auto l2 = sum(squared(edge_normal));
            if (l2 < 1e-12) {
                THROW_OR_ABORT("Cannot compute edge normal");
            }
            edges_(i) = PlaneNd<TData, 3>{ edge_normal / std::sqrt(l2), corners(i) };
        }
    }
    bool contains(const FixedArray<TData, 3>& point) const {
        for (const auto& edge : edges_.flat_iterable()) {
            if (dot0d(edge.normal, point) + edge.intercept < 0) {
                return false;
            }
        }
        return true;
    }
    template <class TDir, class TPos>
    ConvexPolygon3D<TPos, tnvertices> transformed(
        const TransformationMatrix<TDir, TPos, 3>& transformation_matrix) const
    {
        ConvexPolygon3D<TData, tnvertices> result;
        for (size_t i = 0; i < tnvertices; ++i) {
            result.edges_(i) = edges_(i).transformed(transformation_matrix);
        }
        result.plane_ = plane_.transformed(transformation_matrix);
        return result;
    }
    inline const PlaneNd<TData, 3>& plane() const {
        return plane_;
    }
    inline ConvexPolygon3D<TData, tnvertices> operator - () const {
        return { edges_, -plane_ };
    }
private:
    FixedArray<PlaneNd<TData, 3>, tnvertices> edges_;
    PlaneNd<TData, 3> plane_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
