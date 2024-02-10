#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

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

    FixedArray<float, 3> velocity_at_position(const FixedArray<double, 3>& position) const;
    float effective_mass(const VectorAtPosition<float, double, 3>& vp) const;
    void integrate_impulse(const VectorAtPosition<float, double, 3>& J, float extra_w = 0.f);
private:
    const RigidBodyPulses& vehicle_;
    RigidBodyPulses& wheel_;
    FixedArray<float, 3> vertical_line_;
};

}
