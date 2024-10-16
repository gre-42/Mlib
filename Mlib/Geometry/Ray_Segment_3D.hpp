#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Pos.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class PlaneNd;
template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TData, size_t tnvertices>
class ConvexPolygon3D;

template <class TData>
class RaySegment3D {
public:
    RaySegment3D(
        const FixedArray<TData, 3>& start,
        const FixedArray<TData, 3>& direction,
        const TData& length)
        : start{ start }
        , direction{ direction }
        , length{ length }
    {}
    RaySegment3D(
        const FixedArray<TData, 3>& start,
        const FixedArray<TData, 3>& end)
        : start{ start }
        , direction{ uninitialized }
    {
        direction = end - start;
        auto l2 = sum(squared(direction));
        if (l2 < 1e-12) {
            THROW_OR_ABORT("Could not calculate ray direction");
        }
        length = std::sqrt(l2);
        direction /= length;
    }
    explicit RaySegment3D(const FixedArray<TData, 2, 3>& vertices)
    : RaySegment3D{ vertices[0], vertices[1] }
    {}
    bool intersects(const PlaneNd<TData, 3>& plane, TData* t, FixedArray<TData, 3>* intersection_point) const {
        auto c = dot0d(plane.normal, direction);
        if (std::abs(c) < 1e-12) {
            return false;
        }
        *t = -(dot0d(plane.normal, start) + plane.intercept) / c;
        if (((*t) >= 0) && ((*t) <= length)) {
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
    bool intersects_slow(const AxisAlignedBoundingBox<TData, 3>& aabb) const {
        if (aabb.contains(start) || aabb.contains(stop())) {
            return true;
        }
        auto interscts = [&](size_t axis0, size_t axis1, size_t axis2, bool mm) {
            TData t;
            FixedArray<TData, 3> intersection_point;
            PlaneNd<TData, 3> plane;
            plane.normal(axis0) = 1;
            plane.normal(axis1) = 0;
            plane.normal(axis2) = 0;
            if (mm) {
                plane.intercept = -aabb.min(axis0);
            } else {
                plane.intercept = -aabb.max(axis0);
            }
            if (intersects(plane, &t, &intersection_point)) {
                if ((intersection_point(axis1) >= aabb.min(axis1)) && (intersection_point(axis1) <= aabb.max(axis1)) &&
                    (intersection_point(axis2) >= aabb.min(axis2)) && (intersection_point(axis2) <= aabb.max(axis2)))
                {
                    return true;
                }
            }
            return false;
            };
        return
            interscts(0, 1, 2, false) || interscts(1, 2, 0, false) || interscts(2, 0, 1, false) ||
            interscts(0, 1, 2, true) || interscts(1, 2, 0, true) || interscts(2, 0, 1, true);
    }
    FixedArray<TData, 3> stop() const {
        return start + direction * length;
    }
    FixedArray<TData, 3> center() const {
        return start + direction * (length / TData(2));
    }
    RaySegment3D<ScenePos> transformed(
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix) const
    {
        return {
            transformation_matrix.transform(start.template casted<ScenePos>()),
            transformation_matrix.casted<ScenePos, ScenePos>().rotate(direction.template casted<ScenePos>()),
            length
        };
    }
    FixedArray<TData, 3> start;
    FixedArray<TData, 3> direction;
    TData length;
};

template <class TData>
class RaySegment3DForAabb: public RaySegment3D<TData> {
public:
    using RaySegment3D<TData>::start;
    using RaySegment3D<TData>::direction;
    using RaySegment3D<TData>::length;
    using RaySegment3D<TData>::stop;
    using RaySegment3D<TData>::intersects;
    explicit RaySegment3DForAabb(const RaySegment3D<TData>& rs3)
        : RaySegment3D<TData>{ rs3 }
        , inv_direction{ TData(1) / rs3.direction }
    {}
    bool intersects(const AxisAlignedBoundingBox<TData, 3>& aabb) const {
        if (aabb.contains(start) || aabb.contains(stop())) {
            return true;
        }
        auto contains1d = [&](const TData& t, size_t i) {
            auto x = start(i) + direction(i) * t;
            return (x >= aabb.min(i)) && (x <= aabb.max(i));
        };
        auto contains2d = [&](const TData& t, size_t i1, size_t i2) {
            if ((t < 0) || (t > length)) {
                return false;
            }
            return contains1d(t, i1) && contains1d(t, i2);
        };
        auto intersects0 = [&](size_t i0, size_t i1, size_t i2) {
            auto inv_c = inv_direction(i0);
            if (std::abs(inv_c) > 1e12) {
                return false;
            }
            return
                contains2d((aabb.min(i0) - start(i0)) * inv_c, i1, i2) ||
                contains2d((aabb.max(i0) - start(i0)) * inv_c, i1, i2);
        };
        return intersects0(0, 1, 2) || intersects0(1, 2, 0) || intersects0(2, 0, 1);
    }
private:
    FixedArray<TData, 3> inv_direction;
};

}
