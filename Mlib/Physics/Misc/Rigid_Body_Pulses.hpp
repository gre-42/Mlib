#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <iosfwd>

namespace Mlib {

template <class TData, size_t tsize>
struct VectorAtPosition;

struct RigidBodyPulses {

    RigidBodyPulses(
        float mass,
        const FixedArray<float, 3, 3>& I, // inertia tensor
        const FixedArray<float, 3>& com,  // center of mass
        const FixedArray<float, 3>& v,    // velocity
        const FixedArray<float, 3>& w,    // angular velocity
        const FixedArray<float, 3>& position,
        const FixedArray<float, 3>& rotation,
        bool I_is_diagonal);

    FixedArray<float, 3> abs_z() const;
    FixedArray<float, 3> abs_position() const;
    FixedArray<float, 3, 3> abs_I() const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<float, 3>& position) const;
    FixedArray<float, 3> solve_abs_I(const FixedArray<float, 3>& x) const;
    FixedArray<float, 3> transform_to_world_coordinates(const FixedArray<float, 3>& v) const;
    void set_pose(const FixedArray<float, 3, 3>& rotation, const FixedArray<float, 3>& position);
    void integrate_gravity(const FixedArray<float, 3>& g, float dt);
    void integrate_impulse(const VectorAtPosition<float, 3>& J);
    float energy() const;
    float effective_mass(const VectorAtPosition<float, 3>& vp) const;

    void advance_time(float dt);

    float mass_;
    FixedArray<float, 3, 3> I_; // inertia tensor
    FixedArray<float, 3> com_;  // center of mass
    FixedArray<float, 3> v_;    // velocity
    FixedArray<float, 3> w_;    // angular velocity

    FixedArray<float, 3, 3> rotation_;
    FixedArray<float, 3> abs_com_;

    bool I_is_diagonal_;
};

std::ostream& operator << (std::ostream& ostr, const RigidBodyPulses& rbp);

}
