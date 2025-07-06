#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyPulses;
template <class TDir, class TPos, size_t tsize>
struct VectorAtPosition;

class AttachedWheel {
public:
    AttachedWheel(
        const RigidBodyPulses& vehicle,
        RigidBodyPulses& wheel,
        const FixedArray<float, 3>& vertical_line);

    FixedArray<float, 3> velocity_at_position(const FixedArray<ScenePos, 3>& position) const;
    float effective_mass(const VectorAtPosition<float, ScenePos, 3>& vp) const;
    void integrate_impulse(const VectorAtPosition<float, ScenePos, 3>& J, float extra_w, float dt);
private:
    const RigidBodyPulses& vehicle_;
    RigidBodyPulses& wheel_;
    FixedArray<float, 3> vertical_line_;
};

}
