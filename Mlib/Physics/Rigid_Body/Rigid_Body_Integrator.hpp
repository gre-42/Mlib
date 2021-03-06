#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <iosfwd>

namespace Mlib {

template <class TDir, class TPos, size_t tsize>
struct VectorAtPosition;

struct RigidBodyIntegrator {

    RigidBodyIntegrator(
        float mass,
        const FixedArray<float, 3>& L,    // angular momentum
        const FixedArray<float, 3, 3>& I, // inertia tensor
        const FixedArray<float, 3>& com,  // center of mass
        const FixedArray<float, 3>& v,    // velocity
        const FixedArray<float, 3>& w,    // angular velocity
        const FixedArray<float, 3>& T,    // torque
        const FixedArray<double, 3>& position,
        const FixedArray<float, 3>& rotation,
        bool I_is_diagonal);

    FixedArray<float, 3> abs_z() const;
    FixedArray<double, 3> abs_position() const;
    FixedArray<float, 3, 3> abs_I() const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<double, 3>& position) const;
    void set_pose(const FixedArray<float, 3, 3>& rotation, const FixedArray<double, 3>& position);
    void integrate_force(const VectorAtPosition<float, double, 3>& F);
    void integrate_impulse(const VectorAtPosition<float, double, 3>& J);
    void integrate_gravity(const FixedArray<float, 3>& g);
    void reset_forces();
    float energy() const;

    void advance_time(float dt);

    RigidBodyPulses rbp_;
    FixedArray<float, 3> L_;    // angular momentum
    FixedArray<float, 3> a_;    // acceleration
    FixedArray<float, 3> T_;    // torque
};

std::ostream& operator << (std::ostream& ostr, const RigidBodyIntegrator& rbi);

}
