#pragma once
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/ISurface_Normal.hpp>

namespace Mlib {

class RigidBodyVehicle;

class NormalOnBevelBox: public ISurfaceNormal {
public:
	NormalOnBevelBox(
        const DanglingBaseClassRef<RigidBodyVehicle>& rb,
		const AxisAlignedBoundingBox<float, 3>& aabb,
        float radius);
    ~NormalOnBevelBox();
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionRidgeSphere<CompressedScenePos>& ridge,
        const FixedArray<ScenePos, 3>& position) const override;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionPolygonSphere<CompressedScenePos, 3>& triangle,
        const FixedArray<ScenePos, 3>& position) const override;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionPolygonSphere<CompressedScenePos, 4>& quad,
        const FixedArray<ScenePos, 3>& position) const override;
private:
    std::optional<FixedArray<float, 3>> get_surface_normal(
        const FixedArray<ScenePos, 3>& position) const;

    DanglingBaseClassRef<RigidBodyVehicle> rb_;
	AxisAlignedBoundingBox<float, 3> aabb_;
    float radius_;
};

}
