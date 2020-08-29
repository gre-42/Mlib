#include "Rigid_Body_Integrator.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

RigidBodyIntegrator::RigidBodyIntegrator(
    const FixedArray<float, 3>& L,    // angular momentum
    const FixedArray<float, 3, 3>& I, // inertia tensor
    const FixedArray<float, 3>& com,  // center of mass
    const FixedArray<float, 3>& v,    // velocity
    const FixedArray<float, 3>& w,    // angular velocity
    const FixedArray<float, 3>& T,    // torque
    const FixedArray<float, 3>& position,
    const FixedArray<float, 3>& rotation,
    bool I_is_diagonal)
: L_{L},
  I_{I},
  com_{com},
  v_{v},
  w_{w},
  a_{fixed_zeros<float, 3>()},
  T_{T},
  position_{position},
  rotation_{tait_bryan_angles_2_matrix(rotation)},
  I_is_diagonal_{I_is_diagonal}
{}

void RigidBodyIntegrator::advance_time(
    float dt,
    float min_velocity,
    float min_angular_velocity)
{
    v_ += dt * a_;
    L_ += dt * T_;
    if (I_is_diagonal_) {
        // R I R^T w = L
        // => w = R I^{-1} R^T L
        // This enables support for INFINITY inside I.
        FixedArray<float, 3> Iv{I_(0, 0), I_(1, 1), I_(2, 2)};
        w_ = dot1d(rotation_, dot1d(rotation_.T(), L_) / Iv);
    } else {
        w_ = solve_symm_1d(abs_I(), L_);
    }
    if ((sum(squared(v_)) < squared(min_velocity)) &&
        (sum(squared(w_)) < squared(min_angular_velocity)))
    {
        v_ = 0;
        L_ = 0;
        w_ = 0;
    } else {
        position_ += dt * v_;
        rotation_ = dot2d(rodrigues(dt * w_), rotation_);
    }
}

FixedArray<float, 3, 3> RigidBodyIntegrator::abs_I() const {
    return dot2d(rotation_, dot2d(I_, rotation_.T()));
}

FixedArray<float, 3> RigidBodyIntegrator::velocity_at_position(const FixedArray<float, 3>& position) const {
    return v_ + cross(w_, position - position_);
}

FixedArray<float, 3> RigidBodyIntegrator::abs_com() const {
    return dot1d(rotation_, com_) + position_;
}

FixedArray<float, 3> RigidBodyIntegrator::abs_z() const {
    return z3_from_3x3(rotation_);
}
