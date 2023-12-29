#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class PlaneNd;
template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData, size_t tnvertices>
class ConvexPolygon3D;

template <class TData>
class RaySegment3D {
public:
    RaySegment3D(
        const FixedArray<TData, 3>& start,
        const FixedArray<TData, 3>& direction,
        const TData& intercept_end)
        : start{ start }
        , direction{ direction }
        , intercept_end{ intercept_end }
    {}
    explicit RaySegment3D(const FixedArray<FixedArray<TData, 3>, 2>& vertices) {
        direction = vertices(1) - vertices(0);
        auto l2 = sum(squared(direction));
        if (l2 < 1e-12) {
            THROW_OR_ABORT("Could not calculate ray direction");
        }
        direction /= std::sqrt(l2);
        start = vertices(0);
        intercept_end = dot0d(direction, vertices(1));
    }
    bool intersects(const PlaneNd<TData, 3>& plane, TData* t, FixedArray<TData, 3>* intersection_point) const {
        auto c = dot0d(plane.normal, direction);
        if (std::abs(c) < 1e-12) {
            return false;
        }
        *t = -(dot0d(plane.normal, start) + plane.intercept) / c;
        if (((*t) >= 0) && ((*t) <= intercept_end)) {
            *intersection_point = start + direction * (*t);
            return true;
        }
        return false;
    }
    template <size_t tnvertices>
    bool intersects(
        const ConvexPolygon3D<TData, tnvertices>& polygon,
        TData* t,
        FixedArray<TData, 3>* intersection_point) const
    {
        if (!intersects(polygon.plane(), t, intersection_point)) {
            return false;
        }
        return polygon.contains(*intersection_point);
    }
    FixedArray<TData, 3> start;
    FixedArray<TData, 3> direction;
    TData intercept_end;
};

}
