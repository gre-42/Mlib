#include "Rigid_Body_Pulses.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Inverse.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RigidBodyPulses::RigidBodyPulses(
    float mass,
    const FixedArray<float, 3, 3>& I, // inertia tensor
    const FixedArray<float, 3>& com,  // center of mass
    const FixedArray<float, 3>& v,    // velocity
    const FixedArray<float, 3>& w,    // angular velocity
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    bool I_is_diagonal)
    : mass_{ mass }
    , I_{ I }
    , com_{ com }
    , v_{ v }
    , w_{ w }
    , rotation_{ tait_bryan_angles_2_matrix(rotation) }
    , abs_com_{ dot1d(rotation_, com_).casted<ScenePos>() + position }
    , I_is_diagonal_{ I_is_diagonal }
    , abs_I_{ fixed_nans<float, 3, 3>() }
    , abs_I_inv_{ fixed_nans<float, 3, 3>() }
#ifndef NDEBUG
    , abs_I_rotation_{ fixed_nans<float, 3, 3>() }
#endif
{}

void RigidBodyPulses::advance_time(float dt)
{
    abs_com_ += (dt * v_).casted<ScenePos>();
    rotation_ = dot2d(rodrigues1(dt * w_, false), rotation_);  // false = check_angle
    rotation_ = tait_bryan_angles_2_matrix(matrix_2_tait_bryan_angles(rotation_));
    if (!I_is_diagonal_) {
        update_abs_I_and_inv();
    }
    // lerr();
    // lerr() << std::sqrt(sum(squared(v_))) << " "  << (sum(squared(v_)) < squared(min_velocity));
    // lerr() << std::sqrt(sum(squared(w_))) << " "  << (sum(squared(w_)) < squared(min_angular_velocity));
    // lerr() << std::sqrt(sum(squared(a_))) << " "  << (sum(squared(a_)) < squared(min_acceleration));
    // if ((sum(squared(v_)) < squared(min_velocity)) &&
    //     (sum(squared(w_)) < squared(min_angular_velocity)))
    // {
    //     v_ = 0;
    //     w_ = 0;
    // } else {
    //     abs_com_ += dt * v_;
    //     rotation_ = dot2d(rodrigues1(dt * w_), rotation_);
    // }
}

const FixedArray<float, 3, 3>& RigidBodyPulses::abs_I() const {
    assert(all(abs_I_rotation_ == rotation_));
    return abs_I_;
}

const FixedArray<float, 3, 3>& RigidBodyPulses::abs_I_inv() const {
    assert(all(abs_I_rotation_ == rotation_));
    return abs_I_inv_;
}

void RigidBodyPulses::update_abs_I_and_inv() {
    assert(!I_is_diagonal_);
#ifndef NDEBUG
    abs_I_rotation_ = rotation_;
#endif
    abs_I_ = dot2d(rotation_, dot2d(I_, rotation_.T()));
    abs_I_inv_ = fixed_symmetric_inverse_3x3(abs_I());
}

FixedArray<float, 3> RigidBodyPulses::velocity_at_position(const FixedArray<ScenePos, 3>& position) const {
    return v_ + cross(w_, (position - abs_com_).casted<float>());
}

FixedArray<ScenePos, 3> RigidBodyPulses::abs_position() const {
    // abs_com_ = dot1d(rotation_, com_) + position_;
    return abs_com_ - dot1d(rotation_, com_).casted<ScenePos>();
}

TransformationMatrix<float, ScenePos, 3> RigidBodyPulses::abs_transformation() const {
    return TransformationMatrix<float, ScenePos, 3>{ rotation_, abs_position() };
}

FixedArray<ScenePos, 3> RigidBodyPulses::transform_to_world_coordinates(const FixedArray<float, 3>& v) const {
    // return dot1d(rbp.rotation_, v) + rbp.abs_position()
    return dot1d(rotation_, v - com_).casted<ScenePos>() + abs_com_;
}

FixedArray<float, 3> RigidBodyPulses::abs_z() const {
    return z3_from_3x3(rotation_);
}

void RigidBodyPulses::set_pose(const FixedArray<float, 3, 3>& rotation, const FixedArray<ScenePos, 3>& position) {
    rotation_ = rotation;
    abs_com_ = dot1d(rotation_, com_).casted<ScenePos>() + position;
    if (!I_is_diagonal_) {
        update_abs_I_and_inv();
    }
}

FixedArray<float, 3> RigidBodyPulses::solve_abs_I(const FixedArray<float, 3>& x) const
{
    if (I_is_diagonal_) {
        // R I R^T w = L
        // => w = R I^{-1} R^T L
        // This enables support for INFINITY inside I.
        FixedArray<float, 3> Iv{ I_(0, 0), I_(1, 1), I_(2, 2) };
        return dot1d(rotation_, dot(x, rotation_) / Iv);
    } else {
        // return solve_symm_1d(abs_I(), x);
        return dot1d(abs_I_inv(), x);
    }
}

FixedArray<float, 3> RigidBodyPulses::dot1d_abs_I(const FixedArray<float, 3>& x) const
{
    if (I_is_diagonal_) {
        FixedArray<float, 3> Iv{ I_(0, 0), I_(1, 1), I_(2, 2) };
        return dot1d(rotation_, Iv * dot(x, rotation_));
    } else {
        return dot1d(abs_I(), x);
    }
}

void RigidBodyPulses::integrate_delta_v(const FixedArray<float, 3>& dv) {
    v_ += dv;
}

void RigidBodyPulses::integrate_delta_angular_momentum(const FixedArray<float, 3>& dL, float extra_w) {
    w_ += (1 + extra_w) * solve_abs_I(dL);
}

void RigidBodyPulses::integrate_impulse(const VectorAtPosition<float, ScenePos, 3>& J, float extra_w)
{
    if (any(abs(J.vector) > 1e5f)) {
        THROW_OR_ABORT("J.vector out of bounds");
    }
    integrate_delta_v(J.vector / mass_);
    integrate_delta_angular_momentum(cross((J.position - abs_com_).casted<float>(), J.vector), extra_w);
}

float RigidBodyPulses::energy() const {
    // From: http://farside.ph.utexas.edu/teaching/336k/Newtonhtml/node65.html
    return 0.5f * (mass_ * sum(squared(v_)) + dot0d(w_, dot1d_abs_I(w_)));
}

float RigidBodyPulses::effective_mass(const VectorAtPosition<float, ScenePos, 3>& vp) const {
    FixedArray<float, 3> J2 = cross((vp.position - abs_com_).casted<float>(), vp.vector);
    return 1.f / (1.f / mass_ + dot0d(J2, solve_abs_I(J2)));
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyPulses& rbi) {
    ostr << "RigidBodyPulses" << std::endl;
    ostr << "mass " << rbi.mass_ << std::endl;
    ostr << "I " << rbi.I_ << std::endl;
    ostr << "com " << rbi.com_ << std::endl;
    ostr << "v " << rbi.v_ << std::endl;
    ostr << "w " << rbi.w_ << std::endl;
    ostr << "rotation " << rbi.rotation_ << std::endl;
    ostr << "abs_com " << rbi.abs_com_ << std::endl;
    ostr << "I_is_diagonal " << int(rbi.I_is_diagonal_) << std::endl;
    return ostr;
}
