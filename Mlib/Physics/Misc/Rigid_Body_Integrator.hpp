#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <iosfwd>

namespace Mlib {

template <class TData, size_t tsize>
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
        const FixedArray<float, 3>& position,
        const FixedArray<float, 3>& rotation,
        bool I_is_diagonal);

    FixedArray<float, 3> abs_z() const;
    FixedArray<float, 3> abs_position() const;
    FixedArray<float, 3, 3> abs_I() const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<float, 3>& position) const;
    void integrate_force(const VectorAtPosition<float, 3>& F);
    void integrate_gravity(const FixedArray<float, 3>& g);
    void reset_forces();
    float energy() const;

    void advance_time(
        float dt,
        float min_acceleration,
        float min_velocity,
        float min_angular_velocity);

    float mass_;
    FixedArray<float, 3> L_;    // angular momentum
    FixedArray<float, 3, 3> I_; // inertia tensor
    FixedArray<float, 3> com_;  // center of mass
    FixedArray<float, 3> v_;    // velocity
    FixedArray<float, 3> w_;    // angular velocity

    FixedArray<float, 3> a_;    // acceleration
    FixedArray<float, 3> T_;    // torque
    FixedArray<float, 3, 3> rotation_;
    FixedArray<float, 3> abs_com_;

    bool I_is_diagonal_;
};

std::ostream& operator << (std::ostream& ostr, const RigidBodyIntegrator& rbi);

}
