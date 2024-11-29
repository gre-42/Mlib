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
        : edges_{ uninitialized }
        , plane_{ uninitialized }
    {}
    ConvexPolygon3D(
        const FixedArray<PlaneNd<TDir, TPos, 3>, tnvertices>& edges,
        const PlaneNd<TDir, TPos, 3>& plane)
        : edges_{ edges }
        , plane_{ plane }
    {}
    ConvexPolygon3D(const FixedArray<TPos, tnvertices, 3>& corners)
        : edges_{ uninitialized }
        , plane_{ uninitialized }
    {
        static_assert(tnvertices >= 3);
        plane_ = PlaneNd<TDir, TPos, 3>{ FixedArray<TPos, 3, 3>{
            corners[0],
            corners[1],
            corners[2] } };
        for (size_t i = 0; i < tnvertices; ++i) {
            auto dir = corners[(i + 1) % tnvertices] - corners[i];
            auto edge_normal = cross(plane_.normal, dir.template casted<TDir>());
            auto l2 = sum(squared(edge_normal));
            if (l2 < 1e-12) {
                THROW_OR_ABORT("Cannot compute edge normal");
            }
            edges_(i) = PlaneNd<TDir, TPos, 3>{ edge_normal / std::sqrt(l2), corners[i] };
        }
    }
    bool contains(const FixedArray<TPos, 3>& point) const {
        using I = funpack_t<TPos>;
        for (const auto& edge : edges_.flat_iterable()) {
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
            result.edges_(i) = edges_(i).transformed(transformation_matrix);
        }
        result.plane_ = plane_.transformed(transformation_matrix);
        return result;
    }
    inline const PlaneNd<TDir, TPos, 3>& plane() const {
        return plane_;
    }
    inline ConvexPolygon3D<TDir, TPos, tnvertices> operator - () const {
        return { edges_, -plane_ };
    }
    template <class TDir2, class TPos2>
    ConvexPolygon3D<TDir2, TPos2, tnvertices> casted() const {
        return {
            edges_.template applied<PlaneNd<TDir2, TPos2, 3>>([](const auto& plane) { return plane.template casted<TDir2, TPos2>(); }),
            plane_.template casted<TDir2, TPos2>()
        };
    }
private:
    FixedArray<PlaneNd<TDir, TPos, 3>, tnvertices> edges_;
    PlaneNd<TDir, TPos, 3> plane_;
};

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
