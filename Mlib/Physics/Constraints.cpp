#include "Constraints.hpp"
#include <Mlib/Geometry/Arbitrary_Orthogonal.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Physics/Handle_Tire_Triangle_Intersection.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Power_To_Force.hpp>

using namespace Mlib;

NormalContactInfo1::NormalContactInfo1(
    RigidBodyPulses& rbp,
    const BoundedPlaneConstraint& pc,
    const FixedArray<float, 3>& p)
: rbp_{rbp},
  pc_{pc},
  p_{p}
{}

/**
 * From: Erin Catto, Modeling and Solving Constraints
 *       Erin Catto, Fast and Simple Physics using Sequential Impulses
 *       Marijn Tamis, Giuseppe Maggiore, Constraint based physics solver
 *       Marijn Tamis, Sequential Impulse Solver for Rigid Body Dynamics
 */
void NormalContactInfo1::solve(float dt, float relaxation) {
    PlaneConstraint& pc = pc_.constraint;
    if (pc.active(p_)) {
        float v = dot0d(rbp_.velocity_at_position(p_), pc.normal_impulse.normal);
        float mc = rbp_.effective_mass({.vector = pc.normal_impulse.normal, .position = p_});
        float lambda = - mc * (-v + pc.v(p_, dt));
        lambda = pc_.clamped_lambda(relaxation * lambda);
        rbp_.integrate_impulse({
            .vector = -pc.normal_impulse.normal * lambda,
            .position = p_});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
}

NormalContactInfo2::NormalContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const BoundedPlaneConstraint& pc,
    const FixedArray<float, 3>& p)
: rbp0_{rbp0},
  rbp1_{rbp1},
  pc_{pc},
  p_{p}
{}

void NormalContactInfo2::solve(float dt, float relaxation) {
    PlaneConstraint& pc = pc_.constraint;
    if (pc.active(p_)) {
        float v0 = dot0d(rbp0_.velocity_at_position(p_), pc.normal_impulse.normal);
        float v1 = dot0d(rbp1_.velocity_at_position(p_), pc.normal_impulse.normal);
        float mc0 = rbp0_.effective_mass({.vector = pc.normal_impulse.normal, .position = p_});
        float mc1 = rbp1_.effective_mass({.vector = pc.normal_impulse.normal, .position = p_});
        float lambda = - (mc0 * mc1 / (mc0 + mc1)) * (-v0 + v1 + pc.v(p_, dt));
        lambda = pc_.clamped_lambda(relaxation * lambda);
        rbp0_.integrate_impulse({
            .vector = -pc.normal_impulse.normal * lambda,
            .position = p_});
        rbp1_.integrate_impulse({
            .vector = pc.normal_impulse.normal * lambda,
            .position = p_});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
}

FrictionContactInfo1::FrictionContactInfo1(
    RigidBodyPulses& rbp,
    const NormalImpulse& normal_impulse,
    const FixedArray<float, 3>& p,
    float stiction_coefficient,
    float friction_coefficient,
    const FixedArray<float, 3>& b,
    float lateral_stability,
    const FixedArray<float, 3>& clamping_direction,
    float clamping_min,
    float clamping_max,
    float extra_stiction,
    float extra_friction)
: lambda_total_(0),
  b_{b},
  rbp_{rbp},
  normal_impulse_{normal_impulse},
  p_{p},
  stiction_coefficient_{stiction_coefficient},
  friction_coefficient_{friction_coefficient},
  clamping_direction_{clamping_direction},
  clamping_min_{clamping_min},
  clamping_max_{clamping_max},
  lateral_stability_{lateral_stability},
  extra_stiction_{extra_stiction},
  extra_friction_{extra_friction}
{}

void FrictionContactInfo1::solve(float dt, float relaxation) {
    FixedArray<float, 3> v3 = rbp_.velocity_at_position(p_) - b_;
    v3 -= normal_impulse_.normal * dot0d(v3, normal_impulse_.normal);
    if (float vl2 = sum(squared(v3)); vl2 > 1e-12) {
        float v = std::sqrt(vl2);
        FixedArray<float, 3> n3 = v3 / v;
        float mc = rbp_.effective_mass({.vector = n3, .position = p_});
        FixedArray<float, 3> lambda = relaxation * mc * v * n3;
        FixedArray<float, 3> lambda_total_old = lambda_total_;
        lambda_total_ += lambda;
        if (!any(isnan(clamping_direction_))) {
            float ld = dot0d(lambda_total_, clamping_direction_);
            FixedArray<float, 3> lt = lambda_total_ - ld * clamping_direction_;
            if (lateral_stability_ == 0) {
                lambda_total_ = std::clamp(ld, clamping_min_, clamping_max_) * clamping_direction_ + lt;
            } else {
                lambda_total_ =
                    std::clamp(
                        ld,
                        std::max(clamping_min_, -max_impulse_stiction()),
                        std::min(clamping_max_, max_impulse_stiction())) * clamping_direction_ +
                    min_l2(lt, lateral_stability_ * max_impulse_stiction());
            }
        }
        if (float ll2 = sum(squared(lambda_total_)); ll2 > squared(max_impulse_stiction())) {
            lambda_total_ *= max_impulse_friction() / std::sqrt(ll2);
        }
        lambda = lambda_total_ - lambda_total_old;
        rbp_.integrate_impulse({
            .vector = -lambda,
            .position = p_});
    }
}

float FrictionContactInfo1::max_impulse_stiction() const {
    return std::max(0.f, -(stiction_coefficient_ * (1 + extra_stiction_)) * normal_impulse_.lambda_total);
}

float FrictionContactInfo1::max_impulse_friction() const {
    return std::max(0.f, -(friction_coefficient_ * (1 + extra_friction_)) * normal_impulse_.lambda_total);
}

void FrictionContactInfo1::set_b(const FixedArray<float, 3>& b) {
    b_ = b;
}

void FrictionContactInfo1::set_clamping(
    const FixedArray<float, 3>& clamping_direction,
    float clamping_min,
    float clamping_max)
{
    clamping_direction_ = clamping_direction;
    clamping_min_ = clamping_min;
    clamping_max_ = clamping_max;
}

void FrictionContactInfo1::set_extra_friction(
    float extra_stiction,
    float extra_friction)
{
    extra_stiction_ = extra_stiction;
    extra_friction_ = extra_friction;
}
    
std::ostream& Mlib::operator << (std::ostream& ostr, const FrictionContactInfo1& fci1) {
    return ostr << "lambda_total " << std::sqrt(sum(squared(fci1.lambda_total_))) << " | " << fci1.lambda_total_;
}

FrictionContactInfo2::FrictionContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const NormalImpulse& normal_impulse,
    const FixedArray<float, 3>& p,
    float stiction_coefficient,
    float friction_coefficient,
    const FixedArray<float, 3>& b)
: lambda_total_(0),
  b_{b},
  rbp0_{rbp0},
  rbp1_{rbp1},
  normal_impulse_{normal_impulse},
  p_{p},
  stiction_coefficient_{stiction_coefficient},
  friction_coefficient_{friction_coefficient}
{}

void FrictionContactInfo2::solve(float dt, float relaxation) {
    FixedArray<float, 3> v3 = rbp0_.velocity_at_position(p_) - rbp1_.velocity_at_position(p_) - b_;
    v3 -= normal_impulse_.normal * dot0d(v3, normal_impulse_.normal);
    if (float vl2 = sum(squared(v3)); vl2 > 1e-12) {
        float v = std::sqrt(vl2);
        FixedArray<float, 3> n3 = v3 / v;
        float mc0 = rbp0_.effective_mass({.vector = n3, .position = p_});
        float mc1 = rbp1_.effective_mass({.vector = n3, .position = p_});
        FixedArray<float, 3> lambda = relaxation * (mc0 * mc1 / (mc0 + mc1)) * v * n3;
        FixedArray<float, 3> lambda_total_old = lambda_total_;
        lambda_total_ += lambda;
        if (float ll2 = sum(squared(lambda_total_)); ll2 > squared(max_impulse_stiction())) {
            lambda_total_ *= max_impulse_friction() / std::sqrt(ll2);
        }
        lambda = lambda_total_ - lambda_total_old;
        rbp0_.integrate_impulse({
            .vector = -lambda,
            .position = p_});
        rbp1_.integrate_impulse({
            .vector = lambda,
            .position = p_});
    }
}

float FrictionContactInfo2::max_impulse_stiction() const {
    return std::max(0.f, -stiction_coefficient_ * normal_impulse_.lambda_total);
}

float FrictionContactInfo2::max_impulse_friction() const {
    return std::max(0.f, -friction_coefficient_ * normal_impulse_.lambda_total);
}

TireContactInfo1::TireContactInfo1(
    const FrictionContactInfo1& fci,
    RigidBody& rb,
    size_t tire_id,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    float v0,
    const PhysicsEngineConfig& cfg)
: fci_{fci},
  rb_{rb},
  P_{rb.consume_tire_surface_power(tire_id)},
  tire_id_{tire_id},
  v3_{v3},
  n3_{n3},
  v0_{v0},
  cfg_{cfg}
{}

void TireContactInfo1::solve(float dt, float relaxation) {
    float force_min = -INFINITY;
    float force_max = INFINITY;
    FixedArray<float, 3> v = -updated_tire_speed(P_, rb_, v3_, n3_, v0_, fci_.normal_impulse().normal, cfg_, tire_id_, force_min, force_max);
    fci_.set_b(v);
    FixedArray<float, 3> vv = rb_.get_velocity_at_tire_contact(fci_.normal_impulse().normal, tire_id_);
    if (float vvl = sum(squared(vv)); vvl > 1e-12) {
        float ef = std::sqrt(1 - squared(dot0d(vv / std::sqrt(vvl), n3_)));
        ef = std::min(cfg_.max_extra_friction, ef);
        fci_.set_extra_friction(ef, ef);
    } else {
        fci_.set_extra_friction(0, 0);
    }
    fci_.set_clamping(
        n3_,
        force_min * cfg_.dt / cfg_.oversampling,
        force_max * cfg_.dt / cfg_.oversampling);
    fci_.solve(dt, relaxation);
}

void TireContactInfo1::finalize() {
    // std::cerr << "tire id " << tire_id_ << " | " << fci_ << " normal " << fci_.normal_impulse().normal << std::endl;
}

ShockAbsorberContactInfo1::ShockAbsorberContactInfo1(
    RigidBodyPulses& rbp,
    const BoundedShockAbsorberConstraint& sc,
    const FixedArray<float, 3>& p)
: rbp_{rbp},
  sc_{sc},
  p_{p}
{}

void ShockAbsorberContactInfo1::solve(float dt, float relaxation) {
    ShockAbsorberConstraint& sc = sc_.constraint;
    float F = sc.Ks * sc.distance + sc.Ka * dot0d(rbp_.velocity_at_position(p_), sc.normal_impulse.normal);
    float J = sc_.clamped_lambda(relaxation * F * dt);
    rbp_.integrate_impulse({.vector = -sc.normal_impulse.normal * J, .position = p_});
}

void Mlib::solve_contacts(std::list<std::unique_ptr<ContactInfo>>& cis, float dt) {
    for(size_t i = 0; i < 10; ++i) {
        for(const std::unique_ptr<ContactInfo>& ci : cis) {
            ci->solve(dt, i < 1 ? 0.2 : 1);
        }
    }
    for(const std::unique_ptr<ContactInfo>& ci : cis) {
        ci->finalize();
    }
}
