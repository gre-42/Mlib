#pragma once
#include <Mlib/Geometry/Plane_Nd.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TPos, size_t tndim>
class BoundingSphere;

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TDir, class TPos, size_t tnvertices>
class ConvexPolygon3D {
public:
    ConvexPolygon3D(Uninitialized)
        : edges{ uninitialized }
        , plane{ uninitialized }
    {}
    ConvexPolygon3D(
        const FixedArray<PlaneNd<TDir, TPos, 3>, tnvertices>& edges,
        const PlaneNd<TDir, TPos, 3>& plane)
        : edges{ edges }
        , plane{ plane }
    {}
    explicit ConvexPolygon3D(const FixedArray<TPos, tnvertices, 3>& corners)
        : edges{ uninitialized }
        , plane{ uninitialized }
    {
        static_assert(tnvertices >= 3);
        plane = PlaneNd<TDir, TPos, 3>{ FixedArray<TPos, 3, 3>{
            corners[0],
            corners[1],
            corners[2] } };
        for (size_t i = 0; i < tnvertices; ++i) {
            auto dir = corners[(i + 1) % tnvertices] - corners[i];
            auto edge_normal = cross(plane.normal, dir.template casted<TDir>());
            auto l2 = sum(squared(edge_normal));
            if (l2 < 1e-12) {
                THROW_OR_ABORT("Cannot compute edge normal");
            }
            edges(i) = PlaneNd<TDir, TPos, 3>{ edge_normal / std::sqrt(l2), corners[i] };
        }
    }
    bool contains(const FixedArray<TPos, 3>& point) const {
        using I = funpack_t<TPos>;
        for (const auto& edge : edges.flat_iterable()) {
            if (dot0d(edge.normal.template casted<I>(), point.template casted<I>()) + (I)edge.intercept < 0) {
                return false;
            }
        }
        return true;
    }
    ConvexPolygon3D<TDir, TPos, tnvertices> transformed(
        const TransformationMatrix<TDir, TPos, 3>& transformation_matrix) const
    {
        ConvexPolygon3D<TDir, TPos, tnvertices> result = uninitialized;
        for (size_t i = 0; i < tnvertices; ++i) {
            result.edges(i) = edges(i).transformed(transformation_matrix);
        }
        result.plane = plane.transformed(transformation_matrix);
        return result;
    }
    inline ConvexPolygon3D<TDir, TPos, tnvertices> operator - () const {
        return { edges, -plane };
    }
    template <class TDir2, class TPos2>
    ConvexPolygon3D<TDir2, TPos2, tnvertices> casted() const {
        return {
            edges.template applied<PlaneNd<TDir2, TPos2, 3>>([](const auto& plane) { return plane.template casted<TDir2, TPos2>(); }),
            plane.template casted<TDir2, TPos2>()
        };
    }
    bool operator == (const ConvexPolygon3D& other) const {
        return all(edges == other.edges) &&
               (plane == other.plane);
    }
    FixedArray<PlaneNd<TDir, TPos, 3>, tnvertices> edges;
    PlaneNd<TDir, TPos, 3> plane;
};

template <class TDir, class TPos, size_t tnvertices>
ConvexPolygon3D<TDir, TPos, tnvertices> operator + (
    const ConvexPolygon3D<TDir, TPos, tnvertices>& poly,
    const FixedArray<TPos, 3>& x)
{
    auto result = poly;
    for (size_t i = 0; i < tnvertices; ++i) {
        result.edges(i) = result.edges(i) + x;
    }
    result.plane = result.plane + x;
    return result;
}

template <class TDir, class TPos, size_t tnvertices>
ConvexPolygon3D<TDir, TPos, tnvertices> operator - (
    const ConvexPolygon3D<TDir, TPos, tnvertices>& poly,
    const FixedArray<TPos, 3>& x)
{
    return poly + (-x);
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
