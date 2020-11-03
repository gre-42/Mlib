#include "Rigid_Body_Integrator.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

RigidBodyIntegrator::RigidBodyIntegrator(
    float mass,
    const FixedArray<float, 3>& L,    // angular momentum
    const FixedArray<float, 3, 3>& I, // inertia tensor
    const FixedArray<float, 3>& com,  // center of mass
    const FixedArray<float, 3>& v,    // velocity
    const FixedArray<float, 3>& w,    // angular velocity
    const FixedArray<float, 3>& T,    // torque
    const FixedArray<float, 3>& position,
    const FixedArray<float, 3>& rotation,
    bool I_is_diagonal)
: mass_{mass},
  L_{L},
  I_{I},
  com_{com},
  v_{v},
  w_{w},
  a_{fixed_zeros<float, 3>()},
  T_{T},
  rotation_{tait_bryan_angles_2_matrix(rotation)},
  abs_com_{dot1d(rotation_, com_) + position},
  I_is_diagonal_{I_is_diagonal}
{}

void RigidBodyIntegrator::advance_time(
    float dt,
    float min_acceleration,
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
    // std::cerr << std::endl;
    // std::cerr << std::sqrt(sum(squared(v_))) << " "  << (sum(squared(v_)) < squared(min_velocity)) << std::endl;
    // std::cerr << std::sqrt(sum(squared(w_))) << " "  << (sum(squared(w_)) < squared(min_angular_velocity)) << std::endl;
    // std::cerr << std::sqrt(sum(squared(a_))) << " "  << (sum(squared(a_)) < squared(min_acceleration)) << std::endl;
    if ((sum(squared(v_)) < squared(min_velocity)) &&
        (sum(squared(w_)) < squared(min_angular_velocity) &&
        (sum(squared(a_)) < squared(min_acceleration))))
    {
        v_ = 0;
        L_ = 0;
        w_ = 0;
    } else {
        abs_com_ += dt * v_;
        rotation_ = dot2d(rodrigues(dt * w_), rotation_);
    }
}

FixedArray<float, 3, 3> RigidBodyIntegrator::abs_I() const {
    return dot2d(rotation_, dot2d(I_, rotation_.T()));
}

FixedArray<float, 3> RigidBodyIntegrator::velocity_at_position(const FixedArray<float, 3>& position) const {
    return v_ + cross(w_, position - abs_com_);
}

FixedArray<float, 3> RigidBodyIntegrator::abs_position() const {
    // abs_com = dot1d(rotation_, com_) + position_;
    return abs_com_ - dot1d(rotation_, com_);
}

FixedArray<float, 3> RigidBodyIntegrator::abs_z() const {
    return z3_from_3x3(rotation_);
}

void RigidBodyIntegrator::integrate_force(const VectorAtPosition<float, 3>& F)
{
    a_ += F.vector / mass_;
    T_ += cross(F.position - abs_com_, F.vector);
}

void RigidBodyIntegrator::integrate_gravity(const FixedArray<float, 3>& g) {
    a_ += g;
}

float RigidBodyIntegrator::energy() const {
    // From: http://farside.ph.utexas.edu/teaching/336k/Newtonhtml/node65.html
    return 0.5f * (mass_ * sum(squared(v_)) + dot0d(w_, dot1d(abs_I(), w_)));
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyIntegrator& rbi) {
    ostr << "RigidBodyIntegrator" << std::endl;
    ostr << "mass " << rbi.mass_ << std::endl;
    ostr << "L " << rbi.L_ << std::endl;
    ostr << "I " << rbi.I_ << std::endl;
    ostr << "com " << rbi.com_ << std::endl;
    ostr << "v " << rbi.v_ << std::endl;
    ostr << "w " << rbi.w_ << std::endl;
    ostr << "a " << rbi.a_ << std::endl;
    ostr << "T " << rbi.T_ << std::endl;
    ostr << "rotation " << rbi.rotation_ << std::endl;
    ostr << "abs_com " << rbi.abs_com_ << std::endl;
    ostr << "I_is_diagonal " << rbi.I_is_diagonal_ << std::endl;
    return ostr;
}
