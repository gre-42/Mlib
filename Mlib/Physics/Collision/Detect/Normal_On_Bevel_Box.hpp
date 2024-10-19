#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Physics/Interfaces/ISurface_Normal.hpp>

namespace Mlib {

class RigidBodyPulses;

class NormalOnBevelBox: public ISurfaceNormal {
public:
	NormalOnBevelBox(
        const RigidBodyPulses& rbp,
		const AxisAlignedBoundingBox<float, 3>& aabb,
        float radius);
    ~NormalOnBevelBox();
    virtual FixedArray<float, 3> get_surface_normal(
        const CollisionRidgeSphere& ridge,
        const FixedArray<ScenePos, 3>& position) const override;
    virtual FixedArray<float, 3> get_surface_normal(
        const CollisionPolygonSphere<ScenePos, 3>& triangle,
        const FixedArray<ScenePos, 3>& position) const override;
    virtual FixedArray<float, 3> get_surface_normal(
        const CollisionPolygonSphere<ScenePos, 4>& quad,
        const FixedArray<ScenePos, 3>& position) const override;
private:
    FixedArray<float, 3> get_surface_normal(
        const FixedArray<ScenePos, 3>& position) const;

    const RigidBodyPulses& rbp_;
	AxisAlignedBoundingBox<float, 3> aabb_;
    float radius_;
};

}
