#pragma once
#include <Mlib/Physics/Interfaces/ISurface_Normal.hpp>

namespace Mlib {

class RoundMeshNormal: public ISurfaceNormal {
public:
	RoundMeshNormal(float t, float k);
    ~RoundMeshNormal();
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
    float t_;
    float k_;
};

}
