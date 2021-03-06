#include "Rigid_Body_Integrator.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
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
    const FixedArray<double, 3>& position,
    const FixedArray<float, 3>& rotation,
    bool I_is_diagonal)
: rbp_{
    mass,
    I,
    com,
    v,
    w,
    position,
    rotation,
    I_is_diagonal},
  L_{L},
  a_{fixed_zeros<float, 3>()},
  T_{T}
{}

void RigidBodyIntegrator::advance_time(float dt)
{
    rbp_.v_ += dt * a_;
    L_ += dt * T_;
    rbp_.w_ = rbp_.solve_abs_I(L_);
    
    rbp_.abs_com_ += (dt * rbp_.v_).casted<double>();
    rbp_.rotation_ = dot2d(rodrigues1(dt * rbp_.w_), rbp_.rotation_);
}

FixedArray<float, 3, 3> RigidBodyIntegrator::abs_I() const {
    return rbp_.abs_I();
}

FixedArray<float, 3> RigidBodyIntegrator::velocity_at_position(const FixedArray<double, 3>& position) const {
    return rbp_.velocity_at_position(position);
}

FixedArray<double, 3> RigidBodyIntegrator::abs_position() const {
    return rbp_.abs_position();
}

FixedArray<float, 3> RigidBodyIntegrator::abs_z() const {
    return rbp_.abs_z();
}

void RigidBodyIntegrator::set_pose(const FixedArray<float, 3, 3>& rotation, const FixedArray<double, 3>& position) {
    rbp_.set_pose(rotation, position);
}

void RigidBodyIntegrator::integrate_force(const VectorAtPosition<float, double, 3>& F)
{
    a_ += F.vector / rbp_.mass_;
    T_ += cross((F.position - rbp_.abs_com_).casted<float>(), F.vector);
}

void RigidBodyIntegrator::integrate_impulse(const VectorAtPosition<float, double, 3>& J)
{
    rbp_.integrate_impulse(J);
    L_ = rbp_.dot1d_abs_I(rbp_.w_);
}

void RigidBodyIntegrator::integrate_gravity(const FixedArray<float, 3>& g) {
    a_ += g;
}

void RigidBodyIntegrator::reset_forces() {
    a_ = 0.f;
    T_ = 0.f;
}

float RigidBodyIntegrator::energy() const {
    return rbp_.energy();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyIntegrator& rbi) {
    ostr << "RigidBodyIntegrator" << std::endl;
    ostr << rbi.rbp_ << std::endl;
    ostr << "L " << rbi.L_ << std::endl;
    ostr << "a " << rbi.a_ << std::endl;
    ostr << "T " << rbi.T_ << std::endl;
    return ostr;
}
