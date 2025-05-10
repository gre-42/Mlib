#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <iosfwd>

namespace Mlib {

template <class TDir, class TPos, size_t tsize>
struct VectorAtPosition;
class RigidBodyPulses;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TDir, class TPos, size_t n>
struct RotatingFrame;

std::ostream& operator << (std::ostream& ostr, const RigidBodyPulses& rbi);

class RigidBodyPulses {
    friend std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyPulses& rbi);
public:
    RigidBodyPulses(
        float mass,
        const FixedArray<float, 3, 3>& I,   // inertia tensor
        const FixedArray<float, 3>& com,    // center of mass
        const FixedArray<float, 3>& v_com,  // velocity of the center of mass
        const FixedArray<float, 3>& w,      // angular velocity
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<float, 3>& rotation,
        bool I_is_diagonal,
        float vmax,
        float wmax);

    FixedArray<float, 3> abs_z() const;
    FixedArray<ScenePos, 3> abs_position() const;
    TransformationMatrix<float, ScenePos, 3> abs_transformation() const;
    const FixedArray<float, 3, 3>& abs_I() const;
    const FixedArray<float, 3, 3>& abs_I_inv() const;
    RotatingFrame<SceneDir, ScenePos, 3> rotating_frame() const;
    FixedArray<float, 3> velocity() const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<ScenePos, 3>& position) const;
    FixedArray<float, 3> solve_abs_I(const FixedArray<float, 3>& x) const;
    FixedArray<float, 3> dot1d_abs_I(const FixedArray<float, 3>& x) const;
    FixedArray<ScenePos, 3> transform_to_world_coordinates(const FixedArray<float, 3>& v) const;
    void set_pose(const FixedArray<float, 3, 3>& rotation, const FixedArray<ScenePos, 3>& position);
    void integrate_delta_v(const FixedArray<float, 3>& dv);
    void integrate_delta_angular_momentum(const FixedArray<float, 3>& dL, float extra_w = 0.f);
    void integrate_impulse(const VectorAtPosition<float, ScenePos, 3>& J, float extra_w = 0.f);
    float energy() const;
    float effective_mass(const VectorAtPosition<float, ScenePos, 3>& vp) const;

    void advance_time(float dt);

    float mass_;
    FixedArray<float, 3, 3> I_;     // inertia tensor
    FixedArray<float, 3> com_;      // center of mass
    FixedArray<float, 3> v_com_;    // velocity of the center of mass
    FixedArray<float, 3> w_;        // angular velocity

    FixedArray<float, 3, 3> rotation_;
    FixedArray<ScenePos, 3> abs_com_;

private:
    bool I_is_diagonal_;
    void update_abs_I_and_inv();
    FixedArray<float, 3, 3> abs_I_;
    FixedArray<float, 3, 3> abs_I_inv_;
#ifndef NDEBUG
    mutable FixedArray<float, 3, 3> abs_I_rotation_;
#endif
    float vmax_;
    float wmax_;
};

}
