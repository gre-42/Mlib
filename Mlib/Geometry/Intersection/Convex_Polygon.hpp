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

template <class TPos, size_t tnvertices>
class ConvexPolygon3D {
public:
    ConvexPolygon3D(Uninitialized)
        : edges_{ uninitialized }
        , plane_{ uninitialized }
    {}
    ConvexPolygon3D(
        const FixedArray<PlaneNd<TPos, 3>, tnvertices>& edges,
        const PlaneNd<TPos, 3>& plane)
        : edges_{ edges }
        , plane_{ plane }
    {}
    ConvexPolygon3D(const FixedArray<TPos, tnvertices, 3>& corners)
        : edges_{ uninitialized }
        , plane_{ uninitialized }
    {
        static_assert(tnvertices >= 3);
        plane_ = PlaneNd<TPos, 3>{ FixedArray<TPos, 3, 3>{
            corners[0],
            corners[1],
            corners[2] } };
        for (size_t i = 0; i < tnvertices; ++i) {
            auto dir = corners[(i + 1) % tnvertices] - corners[i];
            auto edge_normal = cross(plane_.normal, dir);
            auto l2 = sum(squared(edge_normal));
            if (l2 < 1e-12) {
                THROW_OR_ABORT("Cannot compute edge normal");
            }
            edges_(i) = PlaneNd<TPos, 3>{ edge_normal / std::sqrt(l2), corners[i] };
        }
    }
    bool contains(const FixedArray<TPos, 3>& point) const {
        for (const auto& edge : edges_.flat_iterable()) {
            if (dot0d(edge.normal, point) + edge.intercept < 0) {
                return false;
            }
        }
        return true;
    }
    template <class TDir>
    ConvexPolygon3D<TPos, tnvertices> transformed(
        const TransformationMatrix<TDir, TPos, 3>& transformation_matrix) const
    {
        ConvexPolygon3D<TPos, tnvertices> result = uninitialized;
        for (size_t i = 0; i < tnvertices; ++i) {
            result.edges_(i) = edges_(i).transformed(transformation_matrix);
        }
        result.plane_ = plane_.transformed(transformation_matrix);
        return result;
    }
    inline const PlaneNd<TPos, 3>& plane() const {
        return plane_;
    }
    inline ConvexPolygon3D<TPos, tnvertices> operator - () const {
        return { edges_, -plane_ };
    }
    template <class TData2>
    ConvexPolygon3D<TData2, tnvertices> casted() const {
        return {
            edges_.template applied<PlaneNd<TData2, 3>>([](const auto& plane) { return plane.template casted<TData2>(); }),
            plane_.template casted<TData2>()
        };
    }
private:
    FixedArray<PlaneNd<TPos, 3>, tnvertices> edges_;
    PlaneNd<TPos, 3> plane_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
