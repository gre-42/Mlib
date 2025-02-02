#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Interfaces/ISurface_Normal.hpp>

namespace Mlib {

class RigidBodyPulses;

class NormalOnCapsule: public ISurfaceNormal {
public:
	NormalOnCapsule(
        const RigidBodyPulses& rbp,
		const TransformationMatrix<float, ScenePos, 3>& trafo);
    ~NormalOnCapsule();
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

    const RigidBodyPulses& rbp_;
	TransformationMatrix<float, ScenePos, 3> itrafo_;
	FixedArray<float, 3, 3> rotate_;
};

}
