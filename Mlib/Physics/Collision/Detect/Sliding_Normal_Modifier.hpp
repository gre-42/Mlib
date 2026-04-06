#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/ICollision_Normal_Modifier.hpp>

namespace Mlib {

class RigidBodyVehicle;

class SlidingNormalModifier: public ICollisionNormalModifier {
public:
	SlidingNormalModifier(
        const DanglingBaseClassRef<const RigidBodyVehicle>& rb,
        float fac,
        float max_overlap);
    ~SlidingNormalModifier();
    virtual void modify_collision_normal(
        const FixedArray<ScenePos, 3>& position,
        FixedArray<float, 3>& normal,
        float& overlap) const override;
private:
    DanglingBaseClassRef<const RigidBodyVehicle> rb_;
    float fac_;
    float max_overlap_;
};

}
