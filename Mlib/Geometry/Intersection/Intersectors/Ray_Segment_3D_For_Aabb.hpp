#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>

namespace Mlib {

template <class TDir, class TPos>
class RaySegment3DForAabb: public RaySegment3D<TDir, TPos> {
public:
    using RaySegment3D<TDir, TPos>::start;
    using RaySegment3D<TDir, TPos>::direction;
    using RaySegment3D<TDir, TPos>::length;
    using RaySegment3D<TDir, TPos>::stop;
    using RaySegment3D<TDir, TPos>::intersects;
    explicit RaySegment3DForAabb(const RaySegment3D<TDir, TPos>& rs3)
        : RaySegment3D<TDir, TPos>{ rs3 }
        , inv_direction{ TDir(1) / rs3.direction }
    {}
    bool intersects(const AxisAlignedBoundingBox<TPos, 3>& aabb) const {
        if (aabb.contains(start) || aabb.contains(stop())) {
            return true;
        }
        auto contains1d = [&](const TPos& t, size_t i) {
            auto x = start(i) + direction(i) * t;
            return (x >= aabb.min(i)) && (x <= aabb.max(i));
        };
        auto contains2d = [&](const TPos& t, size_t i1, size_t i2) {
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
    FixedArray<TDir, 3> inv_direction;
};

template <class TDir, class TPos>
inline bool intersects(
    const RaySegment3DForAabb<TDir, TPos>& a,
    const AxisAlignedBoundingBox<TPos, 3>& b)
{
    return a.intersects(b);
}

inline bool intersects(
    const RaySegment3DForAabb<ScenePos, ScenePos>& a,
    const AxisAlignedBoundingBox<CompressedScenePos, 3>& b)
{
    return a.intersects(b.casted<ScenePos>());
}

}
