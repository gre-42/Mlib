#include "Constraints.hpp"
#include <Mlib/Geometry/Arbitrary_Orthogonal.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Physics/Collision/Handle_Tire_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Pulses.hpp>

using namespace Mlib;

NormalContactInfo1::NormalContactInfo1(
    RigidBodyPulses& rbp,
    const BoundedPlaneInequalityConstraint& pc,
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
    PlaneInequalityConstraint& pc = pc_.constraint;
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
    const BoundedPlaneInequalityConstraint& pc,
    const FixedArray<float, 3>& p)
: rbp0_{rbp0},
  rbp1_{rbp1},
  pc_{pc},
  p_{p}
{}

void NormalContactInfo2::solve(float dt, float relaxation) {
    PlaneInequalityConstraint& pc = pc_.constraint;
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
    const FixedArray<float, 3>& clamping_direction,
    float clamping_min,
    float clamping_max,
    float ortho_clamping_max_l2,
    float extra_stiction,
    float extra_friction,
    float extra_w)
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
  ortho_clamping_max_l2_{ortho_clamping_max_l2},
  extra_stiction_{extra_stiction},
  extra_friction_{extra_friction},
  extra_w_{extra_w}
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
            lambda_total_ =
                std::clamp(
                    ld,
                    clamping_min_,
                    clamping_max_) * clamping_direction_ +
                min_l2(
                    lt,
                    ortho_clamping_max_l2_);
        }
        if (float ll2 = sum(squared(lambda_total_)); ll2 > squared(max_impulse_stiction())) {
            lambda_total_ *= max_impulse_friction() / std::sqrt(ll2);
        }
        lambda = lambda_total_ - lambda_total_old;
        rbp_.integrate_impulse({
            .vector = -lambda,
            .position = p_},
            extra_w_);
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
    float clamping_max,
    float ortho_clamping_max_l2)
{
    clamping_direction_ = clamping_direction;
    clamping_min_ = clamping_min;
    clamping_max_ = clamping_max;
    ortho_clamping_max_l2_ = ortho_clamping_max_l2;
}

void FrictionContactInfo1::set_extras(
    float extra_stiction,
    float extra_friction,
    float extra_w)
{
    extra_stiction_ = extra_stiction;
    extra_friction_ = extra_friction;
    extra_w_ = extra_w;
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
    const FixedArray<float, 3>& vc,
    const FixedArray<float, 3>& n3,
    float v0,
    const PhysicsEngineConfig& cfg)
: fci_{fci},
  rb_{rb},
  P_{rb.consume_tire_surface_power(tire_id)},
  tire_id_{tire_id},
  vc_{vc},
  n3_{n3},
  v0_{v0},
  cfg_{cfg}
{}

void TireContactInfo1::solve(float dt, float relaxation) {
    float force_min;
    float force_max;
    FixedArray<float, 3> tv = updated_tire_speed(P_, rb_, vc_, n3_, v0_, fci_.normal_impulse().normal, cfg_, tire_id_, force_min, force_max);
    // {
    //     FixedArray<float, 3> v3 = rb_.rbi_.rbp_.velocity_at_position(rb_.get_abs_tire_contact_position(tire_id_)) - v;
    //     v3 -= fci_.normal_impulse().normal * dot0d(v3, fci_.normal_impulse().normal);
    //     std::cerr << tire_id_ << " v " << std::sqrt(sum(squared(v))) << " " << dot0d(n3_, v3) << std::endl;
    // }
    // FixedArray<float, 3> x3{
    //     rb_.rbi_.rbp_.rotation_(0, 0),
    //     rb_.rbi_.rbp_.rotation_(1, 0),
    //     rb_.rbi_.rbp_.rotation_(2, 0)};
    // x3 -= fci_.normal_impulse().normal * dot0d(fci_.normal_impulse().normal, x3);
    // x3 /= std::sqrt(sum(squared(x3)));
    // fci_.set_b(v - 1000.f * x3 * rb_.tires_.at(tire_id_).accel_x * (cfg_.dt / cfg_.oversampling));
    fci_.set_b(-tv);
    FixedArray<float, 3> vv = rb_.get_velocity_at_tire_contact(fci_.normal_impulse().normal, tire_id_);
    float slip;
    {
        float vvx = dot0d(vv, n3_);
        float tvx = dot0d(tv, n3_);
        slip = (vvx + tvx) / std::max(cfg_.hand_break_velocity, std::abs(vvx));
    }
    float sin_lateral_slip_angle;
    if (float vvl2 = sum(squared(vv)); vvl2 > 1e-12) {
        sin_lateral_slip_angle = std::sqrt(std::max(0.f, 1 - squared(dot0d(vv / std::sqrt(vvl2), n3_))));
        float ef = std::min(cfg_.max_extra_friction, sin_lateral_slip_angle);
        float ew = std::min(cfg_.max_extra_w, sin_lateral_slip_angle);
        fci_.set_extras(ef, ef, ew);
    } else {
        fci_.set_extras(0, 0, 0);
        sin_lateral_slip_angle = 0;
    }
    float lambda_max =
        (-fci_.normal_impulse().lambda_total) *
        rb_.tires_.at(tire_id_).stiction_coefficient(
            -fci_.normal_impulse().lambda_total / cfg_.dt * cfg_.oversampling);
    FixedArray<float, 2> r = rb_.tires_.at(tire_id_).magic_formula(
        {
            slip,
            std::asin(sin_lateral_slip_angle)},
        cfg_.no_slip ? MagicFormulaMode::NO_SLIP : MagicFormulaMode::STANDARD) * lambda_max;
    // std::cerr << tire_id_ << " | " << r << " | " << dwdt << " | " << vv << " | " << v << std::endl;
    fci_.set_clamping(
        n3_,
        std::max(force_min * cfg_.dt / cfg_.oversampling, -std::abs(r(0))),
        std::min(force_max * cfg_.dt / cfg_.oversampling, std::abs(r(0))),
        std::abs(r(1)));
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
        // std::cerr << "solve_contacts " << i << std::endl;
        for(const std::unique_ptr<ContactInfo>& ci : cis) {
            ci->solve(dt, i < 1 ? 0.2 : 1);
        }
    }
    for(const std::unique_ptr<ContactInfo>& ci : cis) {
        ci->finalize();
    }
}
