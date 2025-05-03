#include "Constraints.hpp"
#include <Mlib/Geometry/Arbitrary_Orthogonal.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Actuators/Velocity_Classification.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Collision/Resolve/Handle_Tire_Triangle_Intersection.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Attached_Wheel.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

template <class TRigidBodyPulsesArg, class TRigidBodyPulsesField>
GenericNormalContactInfo1<TRigidBodyPulsesArg, TRigidBodyPulsesField>::GenericNormalContactInfo1(
    TRigidBodyPulsesArg rbp,
    const BoundedPlaneInequalityConstraint& pc,
    const FixedArray<ScenePos, 3>& p)
    : rbp_{ rbp }
    , pc_{ pc }
    , p_{ p }
{}

/**
 * From: Erin Catto, Modeling and Solving Constraints
 *       Erin Catto, Fast and Simple Physics using Sequential Impulses
 *       Marijn Tamis, Giuseppe Maggiore, Constraint based physics solver
 *       Marijn Tamis, Sequential Impulse Solver for Rigid Body Dynamics
 */
template <class TRigidBodyPulsesArg, class TRigidBodyPulsesField>
void GenericNormalContactInfo1<TRigidBodyPulsesArg, TRigidBodyPulsesField>::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    PlaneInequalityConstraint& pc = pc_.constraint;
    auto snormal = pc.normal_impulse.normal.casted<float>();
    float v = dot0d(rbp_.velocity_at_position(p_), snormal);
    float mc = rbp_.effective_mass({ .vector = snormal, .position = p_ });
    float lambda = - mc * (-v + pc.v(dt));
    lambda = pc_.clamped_lambda(relaxation * lambda);
    rbp_.integrate_impulse({
        .vector = -snormal * lambda,
        .position = p_});
    // linfo() << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x);
}

NormalContactInfo2::NormalContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const BoundedPlaneInequalityConstraint& pc,
    const FixedArray<ScenePos, 3>& p,
    const std::function<void(float)>& notify_lambda_final)
    : rbp0_{ rbp0 }
    , rbp1_{ rbp1 }
    , pc_{ pc }
    , p_{ p }
    , notify_lambda_final_{ notify_lambda_final }
{}

void NormalContactInfo2::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    PlaneInequalityConstraint& pc = pc_.constraint;
    auto snormal = pc.normal_impulse.normal.casted<float>();
    float v0 = dot0d(rbp0_.velocity_at_position(p_), snormal);
    float v1 = dot0d(rbp1_.velocity_at_position(p_), snormal);
    float mc0 = rbp0_.effective_mass({ .vector = snormal, .position = p_ });
    float mc1 = rbp1_.effective_mass({ .vector = snormal, .position = p_ });
    float lambda = - (mc0 * mc1 / (mc0 + mc1)) * (-v0 + v1 + pc.v(dt));
    lambda = pc_.clamped_lambda(relaxation * lambda);
    rbp0_.integrate_impulse({
        .vector = -snormal * lambda,
        .position = p_});
    rbp1_.integrate_impulse({
        .vector = snormal * lambda,
        .position = p_});
    // lerr() << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x);
}

void NormalContactInfo2::finalize() {
    notify_lambda_final_(pc_.constraint.normal_impulse.lambda_total);
}

template <size_t tnullspace>
GenericLineContactInfo1<tnullspace>::GenericLineContactInfo1(
    RigidBodyPulses& rbp0,
    const FixedArray<float, 3>& v1,
    const GenericLineEqualityConstraint<tnullspace>& lec)
    : rbp0_{ rbp0 }
    , v1_{ v1 }
    , lec_{ lec }
{}

template <size_t tnullspace>
void GenericLineContactInfo1<tnullspace>::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    FixedArray<float, 3> v0 = rbp0_.velocity_at_position(lec_.pec.p0);
    FixedArray<float, 3> dv = -v0 + v1_ + lec_.pec.v(dt);
    if constexpr (tnullspace > 0) {
        for (const auto& line_direction : lec_.null_space.row_iterable()) {
            dv -= dot0d(dv, line_direction) * line_direction;
        }
    }
    float len2 = sum(squared(dv));
    if (len2 > 1e-12) {
        FixedArray<float, 3> n = dv / std::sqrt(len2);
        float mc0 = rbp0_.effective_mass({ .vector = n, .position = lec_.pec.p0 });
        FixedArray<float, 3> lambda = - relaxation * mc0 * dv;
        rbp0_.integrate_impulse({
            .vector = -lambda,
            .position = lec_.pec.p0});
    }
}

template <size_t tnullspace>
GenericLineContactInfo2<tnullspace>::GenericLineContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const GenericLineEqualityConstraint<tnullspace>& lec)
    : rbp0_{ rbp0 }
    , rbp1_{ rbp1 }
    , lec_{ lec }
{}

template <size_t tnullspace>
void GenericLineContactInfo2<tnullspace>::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    FixedArray<float, 3> v0 = rbp0_.velocity_at_position(lec_.pec.p0);
    FixedArray<float, 3> v1 = rbp1_.velocity_at_position(lec_.pec.p1);
    FixedArray<float, 3> dv = -v0 + v1 + lec_.pec.v(dt);
    if constexpr (tnullspace > 0) {
        for (const auto& line_direction : lec_.null_space.row_iterable()) {
            dv -= dot0d(dv, line_direction) * line_direction;
        }
    }
    float len2 = sum(squared(dv));
    if (len2 > 1e-12) {
        FixedArray<float, 3> n = dv / std::sqrt(len2);
        float mc0 = rbp0_.effective_mass({ .vector = n, .position = lec_.pec.p0 });
        float mc1 = rbp1_.effective_mass({ .vector = n, .position = lec_.pec.p1 });
        FixedArray<float, 3> lambda = - relaxation * (mc0 * mc1 / (mc0 + mc1)) * dv;
        rbp0_.integrate_impulse({
            .vector = -lambda,
            .position = lec_.pec.p0});
        rbp1_.integrate_impulse({
            .vector = lambda,
            .position = lec_.pec.p1});
    }
}

PlaneContactInfo1::PlaneContactInfo1(
    RigidBodyPulses& rbp0,
    const FixedArray<float, 3>& v1,
    const BoundedPlaneEqualityConstraint& pec)
    : rbp0_{ rbp0 }
    , v1_{ v1 }
    , pec_{ pec }
{}

void PlaneContactInfo1::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    auto& pec = pec_.constraint;
    FixedArray<float, 3> v0 = rbp0_.velocity_at_position(pec.pec.p0);
    FixedArray<float, 3> dv = -v0 + v1_ + pec.pec.v(dt);
    float dv_len = dot0d(dv, pec.plane_normal);
    float mc0 = rbp0_.effective_mass({ .vector = pec.plane_normal, .position = pec.pec.p0 });
    float lambda = - mc0 * dv_len;
    lambda = pec_.clamped_lambda(relaxation * lambda);
    rbp0_.integrate_impulse({
        .vector = - pec.plane_normal * lambda,
        .position = pec.pec.p0});
}

PlaneContactInfo2::PlaneContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const BoundedPlaneEqualityConstraint& pec)
    : rbp0_{ rbp0 }
    , rbp1_{ rbp1 }
    , pec_{ pec }
{}

void PlaneContactInfo2::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    auto& pec = pec_.constraint;
    FixedArray<float, 3> v0 = rbp0_.velocity_at_position(pec.pec.p0);
    FixedArray<float, 3> v1 = rbp1_.velocity_at_position(pec.pec.p1);
    FixedArray<float, 3> dv = -v0 + v1 + pec.pec.v(dt);
    float dv_len = dot0d(dv, pec.plane_normal);
    float mc0 = rbp0_.effective_mass({ .vector = pec.plane_normal, .position = pec.pec.p0 });
    float mc1 = rbp1_.effective_mass({ .vector = pec.plane_normal, .position = pec.pec.p1 });
    float lambda = - (mc0 * mc1 / (mc0 + mc1)) * dv_len;
    lambda = pec_.clamped_lambda(relaxation * lambda);
    rbp0_.integrate_impulse({
        .vector = - pec.plane_normal * lambda,
        .position = pec.pec.p0});
    rbp1_.integrate_impulse({
        .vector = pec.plane_normal * lambda,
        .position = pec.pec.p1});
}

FrictionContactInfo1::FrictionContactInfo1(
    RigidBodyPulses& rbp,
    const NormalImpulse& normal_impulse,
    const FixedArray<ScenePos, 3>& p,
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
    : lambda_total_(0.f)
    , b_{ b }
    , rbp_{ rbp }
    , normal_impulse_{ normal_impulse }
    , p_{ p }
    , stiction_coefficient_{ stiction_coefficient }
    , friction_coefficient_{ friction_coefficient }
    , clamping_direction_{ clamping_direction }
    , clamping_min_{ clamping_min }
    , clamping_max_{ clamping_max }
    , ortho_clamping_max_l2_{ ortho_clamping_max_l2 }
    , extra_stiction_{ extra_stiction }
    , extra_friction_{ extra_friction }
    , extra_w_{ extra_w }
{}

void FrictionContactInfo1::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    FixedArray<float, 3> v3 = rbp_.velocity_at_position(p_) - b_;
    auto snormal = normal_impulse_.normal.casted<float>();
    v3 -= snormal * dot0d(v3, snormal);
    if (float vl2 = sum(squared(v3)); vl2 > 1e-12) {
        float v = std::sqrt(vl2);
        FixedArray<float, 3> n3 = v3 / v;
        float mc = rbp_.effective_mass({ .vector = n3, .position = p_ });
        FixedArray<float, 3> lambda = relaxation * mc * v * n3;
        FixedArray<float, 3> lambda_total_old = lambda_total_;
        lambda_total_ += lambda;
        if (!any(Mlib::isnan(clamping_direction_))) {
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
        if (std::isnan(stiction_coefficient_) != std::isnan(friction_coefficient_)) {
            THROW_OR_ABORT("Differing stiction/friction NaN-ness");
        }
        if (!std::isnan(stiction_coefficient_)) {
            if (float ll2 = sum(squared(lambda_total_)); ll2 > squared(max_impulse_stiction())) {
                lambda_total_ *= max_impulse_friction() / std::sqrt(ll2);
            }
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

const FixedArray<float, 3>& FrictionContactInfo1::get_b() const {
    return b_;
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
    if ((clamping_min > clamping_max) ||
        (std::abs(clamping_min) > float{ 1e4 }) ||
        (std::abs(clamping_max) > float{ 1e4 }))
    {
        THROW_OR_ABORT("FrictionContactInfo1::set_clamping: clamping out of bounds");
    }
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
    const FixedArray<ScenePos, 3>& p,
    float stiction_coefficient,
    float friction_coefficient,
    const FixedArray<float, 3>& b)
    : lambda_total_(0.f)
    , b_{ b }
    , rbp0_{ rbp0 }
    , rbp1_{ rbp1 }
    , normal_impulse_{ normal_impulse }
    , p_{ p }
    , stiction_coefficient_{ stiction_coefficient }
    , friction_coefficient_{ friction_coefficient }
{}

void FrictionContactInfo2::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    FixedArray<float, 3> v3 = rbp0_.velocity_at_position(p_) - rbp1_.velocity_at_position(p_) - b_;
    auto snormal = normal_impulse_.normal.casted<float>();
    v3 -= snormal * dot0d(v3, snormal);
    if (float vl2 = sum(squared(v3)); vl2 > 1e-12) {
        float v = std::sqrt(vl2);
        FixedArray<float, 3> n3 = v3 / v;
        float mc0 = rbp0_.effective_mass({ .vector = n3, .position = p_ });
        float mc1 = rbp1_.effective_mass({ .vector = n3, .position = p_ });
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

void FrictionContactInfo2::set_b(const FixedArray<float, 3>& b) {
    b_ = b;
}

TireContactInfo1::TireContactInfo1(
    const FrictionContactInfo1& fci,
    float surface_stiction_factor,
    RigidBodyVehicle& rb,
    size_t tire_id,
    const FixedArray<float, 3>& vc_street,
    const FixedArray<float, 3>& vc,
    const FixedArray<float, 3>& n3,
    float v0,
    const PhysicsEngineConfig& cfg)
    : fci_{ fci }
    , surface_stiction_factor_{ surface_stiction_factor }
    , rb_{ rb }
    , P_{ rb.consume_tire_surface_power(
        tire_id,
        std::abs(v0) > cfg.hand_brake_velocity
            ? VelocityClassification::FAST
            : VelocityClassification::SLOW) }
    , tire_id_{ tire_id }
    , vc_street_{ vc_street }
    , vc_{ vc }
    , n3_{ n3 }
    , v0_{ v0 }
    , b0_{ fci.get_b() }
    , cfg_{ cfg }
{}

void TireContactInfo1::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    if (rb_.grind_state_.grinding_) {
        return;
    }
    float force_min;
    float force_max;
    handle_tire_triangle_intersection(
        P_,
        rb_,
        b0_,
        vc_street_,
        vc_,
        n3_,
        v0_,
        fci_.normal_impulse().normal.casted<float>(),
        cfg_,
        tire_id_,
        force_min,
        force_max);

    const auto& tire = rb_.tires_.get(tire_id_);

    float tv_len = rb_.get_tire_angular_velocity(tire_id_) * rb_.get_tire_radius(tire_id_);
    FixedArray<float, 3> tv = n3_ * tv_len;

    // {
    //     FixedArray<float, 3> v3 = rb_.rbp_.velocity_at_position(rb_.get_abs_tire_contact_position(tire_id_)) - v;
    //     v3 -= fci_.normal_impulse().normal * dot0d(v3, fci_.normal_impulse().normal);
    //     lerr() << tire_id_ << " v " << std::sqrt(sum(squared(v))) << " " << dot0d(n3_, v3);
    // }
    // FixedArray<float, 3> x3{
    //     rb_.rbp_.rotation_(0, 0),
    //     rb_.rbp_.rotation_(1, 0),
    //     rb_.rbp_.rotation_(2, 0)};
    // x3 -= fci_.normal_impulse().normal * dot0d(fci_.normal_impulse().normal, x3);
    // x3 /= std::sqrt(sum(squared(x3)));
    // fci_.set_b(v - 1000.f * x3 * tire.accel_x * cfg_.dt_substeps());
    fci_.set_b(b0_ - tv);
    FixedArray<float, 3> vv = rb_.get_velocity_at_tire_contact(fci_.normal_impulse().normal.casted<float>(), tire_id_) - b0_;
    float slip;
    {
        // slip = ((vehicle velocity=vvx) + (tire velocity=tvx)) / (vehicle velocity=vvx)
        float vvx = dot0d(vv, n3_);
        float tvx = dot0d(tv, n3_);
        slip = (vvx + tvx) / std::max(cfg_.hand_brake_velocity, std::abs(vvx));
    }
    float sin_lateral_slip_angle;
    // Reference implementation (sin_lateral_slip_angle):
    // {
    //     float vvx = dot0d(vv, n3_);
    //     auto vvt = vv - n3_ * vvx;
    //     auto vvv = vvt + n3_ * (std::abs(vvx) + std::max(0.f, cfg_.hand_brake_velocity - std::abs(vvx)));
    //     auto vvn = vvv / std::sqrt(sum(squared(vvv)));
    //     sin_lateral_slip_angle = std::sqrt(std::max(0.f, 1 - squared(dot0d(vvn, n3_))));
    // }
    // Optimized implementation (sin_lateral_slip_angle):
    {
        float vvx = dot0d(vv, n3_);
        auto ccc = std::max(0.f, sum(squared(vv)) - squared(vvx));
        auto bbb = squared(std::abs(vvx) + std::max(0.f, cfg_.hand_brake_velocity - std::abs(vvx)));
        sin_lateral_slip_angle = std::sqrt(std::max(0.f, 1 - bbb / (ccc + bbb)));
    }
    {
        float ef = std::min(cfg_.max_extra_friction, sin_lateral_slip_angle);
        float ew = std::min(cfg_.max_extra_w, sin_lateral_slip_angle);
        fci_.set_extras(ef, ef, ew);
    }
    float lambda_max =
        (-fci_.normal_impulse().lambda_total) *
        tire.stiction_coefficient(
            -fci_.normal_impulse().lambda_total / cfg_.dt_substeps()) *
        surface_stiction_factor_;
    FixedArray<float, 2> r = tire.magic_formula(
        {
            slip,
            std::asin(sin_lateral_slip_angle)},
        cfg_.no_slip ? MagicFormulaMode::NO_SLIP : MagicFormulaMode::STANDARD) * lambda_max;
    // lerr() << tire_id_ << " | " << r;
    fci_.set_clamping(
        n3_,
        signed_min(force_min * cfg_.dt_substeps(), std::abs(r(0))),
        signed_min(force_max * cfg_.dt_substeps(), std::abs(r(0))),
        std::abs(r(1)));
    fci_.solve(dt, relaxation, iteration, niterations);
}

// void TireContactInfo1::finalize() {
//     lerr() << "tire id " << tire_id_ << " | " << fci_ << " normal " << fci_.normal_impulse().normal;
// }

ShockAbsorberContactInfo1::ShockAbsorberContactInfo1(
    RigidBodyPulses& rbp,
    const BoundedShockAbsorberConstraint& sc,
    const FixedArray<ScenePos, 3>& p)
    : rbp_{ rbp }
    , sc_{ sc }
    , p_{ p }
{}

void ShockAbsorberContactInfo1::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    ShockAbsorberConstraint& sc = sc_.constraint;
    float F = sc.Ks * sc.distance + sc.Ka *
        dot0d(
            rbp_.velocity_at_position(p_),
            sc.normal_impulse.normal.casted<float>());
    float J = sc_.clamped_lambda(1.f / (float)niterations * F * dt);
    rbp_.integrate_impulse({
        .vector = -sc.normal_impulse.normal.casted<float>() * sc.fit * J,
        .position = p_ });
}

ShockAbsorberContactInfo2::ShockAbsorberContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const BoundedShockAbsorberConstraint& sc,
    const FixedArray<ScenePos, 3>& p)
    : rbp0_{ rbp0 }
    , rbp1_{ rbp1 }
    , sc_{ sc }
    , p_{ p }
{}

void ShockAbsorberContactInfo2::solve(float dt, float relaxation, size_t iteration, size_t niterations) {
    ShockAbsorberConstraint& sc = sc_.constraint;
    float F = sc.Ks * sc.distance + sc.Ka *
        dot0d(
            rbp1_.velocity_at_position(p_) - rbp0_.velocity_at_position(p_),
            sc.normal_impulse.normal.casted<float>());
    float J = sc_.clamped_lambda(1.f / (float)niterations * F * dt);
    auto lambda = sc.normal_impulse.normal.casted<float>() * J;
    rbp0_.integrate_impulse({
        .vector = lambda,
        .position = p_ });
    rbp1_.integrate_impulse({
        .vector = -lambda,
        .position = p_ });
}

void Mlib::solve_contacts(std::list<std::unique_ptr<IContactInfo>>& cis, float dt) {
    size_t niterations = 5;
    for (size_t i = 0; i < niterations; ++i) {
        // linfo() << "solve_contacts " << i;
        for (const auto& ci : cis) {
            ci->solve(dt, i < 1 ? 0.2f : 1.f, i, niterations);
        }
    }
    for (const auto& ci : cis) {
        ci->finalize();
    }
}

namespace Mlib {

template class GenericLineContactInfo1<0>;
template class GenericLineContactInfo1<1>;
template class GenericLineContactInfo2<0>;
template class GenericLineContactInfo2<1>;
template class GenericNormalContactInfo1<RigidBodyPulses&, RigidBodyPulses&>;
template class GenericNormalContactInfo1<const AttachedWheel&, AttachedWheel>;

}
