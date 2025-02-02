#pragma once
#include <Mlib/Physics/Interfaces/ICollision_Normal_Modifier.hpp>

namespace Mlib {

class RigidBodyPulses;

class SlidingNormalModifier: public ICollisionNormalModifier {
public:
	SlidingNormalModifier(
        const RigidBodyPulses& rbp,
        float fac,
        float max_overlap);
    ~SlidingNormalModifier();
    virtual void modify_collision_normal(
        const FixedArray<ScenePos, 3>& position,
        FixedArray<float, 3>& normal,
        float& overlap) const override;
private:
    const RigidBodyPulses& rbp_;
    float fac_;
    float max_overlap_;
};

}
