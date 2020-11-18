#include "Constraints.hpp"
#include <Mlib/Geometry/Arbitrary_Orthogonal.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Physics/Handle_Tire_Triangle_Intersection.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Pulses.hpp>

using namespace Mlib;

ContactInfo1::ContactInfo1(
    RigidBodyPulses& rbp,
    const PlaneConstraint& pc,
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
void ContactInfo1::solve(float dt, float relaxation) {
    if (pc_.active(p_)) {
        float v = dot0d(rbp_.velocity_at_position(p_), pc_.plane.normal_);
        float mc = rbp_.effective_mass({.vector = pc_.plane.normal_, .position = p_});
        float lambda = - mc * (-v + pc_.v(p_, dt));
        lambda = pc_.clamped_lambda(relaxation * lambda);
        rbp_.integrate_impulse({
            .vector = -pc_.plane.normal_ * lambda,
            .position = p_});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
}

ContactInfo2::ContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const PlaneConstraint& pc,
    const FixedArray<float, 3>& p)
: rbp0_{rbp0},
  rbp1_{rbp1},
  pc_{pc},
  p_{p}
{}

void ContactInfo2::solve(float dt, float relaxation) {
    if (pc_.active(p_)) {
        float v0 = dot0d(rbp0_.velocity_at_position(p_), pc_.plane.normal_);
        float v1 = dot0d(rbp1_.velocity_at_position(p_), pc_.plane.normal_);
        float mc0 = rbp0_.effective_mass({.vector = pc_.plane.normal_, .position = p_});
        float mc1 = rbp1_.effective_mass({.vector = pc_.plane.normal_, .position = p_});
        float lambda = - (mc0 * mc1 / (mc0 + mc1)) * (-v0 + v1 + pc_.v(p_, dt));
        lambda = pc_.clamped_lambda(relaxation * lambda);
        rbp0_.integrate_impulse({
            .vector = -pc_.plane.normal_ * lambda,
            .position = p_});
        rbp1_.integrate_impulse({
            .vector = pc_.plane.normal_ * lambda,
            .position = p_});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
}

OrthoPlaneConstraints::OrthoPlaneConstraints(
    const FixedArray<float, 3>& normal,
    const FixedArray<float, 3>& point_on_plane,
    const FixedArray<float, 3>& b)
{
    FixedArray<float, 3> t0 = arbitrary_orthogonal(normal);
    t0 /= std::sqrt(sum(squared(t0)));
    FixedArray<float, 3> t1 = cross(t0, normal);
    pcs_[0].plane = PlaneNd<float, 3>{t0, point_on_plane};
    pcs_[1].plane = PlaneNd<float, 3>{t1, point_on_plane};
    set_b(b);
}

void OrthoPlaneConstraints::set_b(const FixedArray<float, 3>& b) {
    pcs_[0].b = dot0d(pcs_[0].plane.normal_, b);
    pcs_[1].b = dot0d(pcs_[1].plane.normal_, b);
}

FrictionContactInfo1::FrictionContactInfo1(
    RigidBodyPulses& rbp,
    const PlaneConstraint& normal_constraint,
    const FixedArray<float, 3>& p,
    float stiction_coefficient,
    float friction_coefficient,
    const FixedArray<float, 3>& b)
: OrthoPlaneConstraints{normal_constraint.plane.normal_, p, b},
  rbp_{rbp},
  normal_constraint_{normal_constraint},
  p_{p},
  stiction_coefficient_{stiction_coefficient},
  friction_coefficient_{friction_coefficient}
{}

void FrictionContactInfo1::solve(float dt, float relaxation) {
    auto get_lambda = [this, dt](const PlaneConstraint& pc) {
        if (pc.active(p_)) {
            float v = dot0d(rbp_.velocity_at_position(p_), pc.plane.normal_);
            float mc = rbp_.effective_mass({.vector = pc.plane.normal_, .position = p_});
            return - mc * (-v + pc.v(p_, dt));
        } else {
            return 0.f;
        }
    };
    FixedArray<float, 2> lambda_total_old = {pcs_[0].lambda_total, pcs_[1].lambda_total};
    FixedArray<float, 2> lambda = {get_lambda(pcs_[0]), get_lambda(pcs_[1])};
    FixedArray<float, 2> lambda_total_new = lambda_total_old + lambda;
    if (float l2 = sum(squared(lambda_total_new)); l2 > squared(max_impulse())) {
        lambda_total_new *= max_impulse() / std::sqrt(l2);
    }
    auto apply_lambda = [this](PlaneConstraint& pc, float lambda) {
        if (pc.active(p_)) {
            pc.lambda_total += lambda;
            rbp_.integrate_impulse({
                .vector = -pc.plane.normal_ * lambda,
                .position = p_});
        }
    };
    apply_lambda(pcs_[0], lambda_total_new(0) - lambda_total_old(0));
    apply_lambda(pcs_[1], lambda_total_new(1) - lambda_total_old(1));
}

FrictionContactInfo2::FrictionContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const PlaneConstraint& normal_constraint,
    const FixedArray<float, 3>& p,
    float stiction_coefficient,
    float friction_coefficient,
    const FixedArray<float, 3>& b)
: OrthoPlaneConstraints{normal_constraint.plane.normal_, p, b},
  rbp0_{rbp0},
  rbp1_{rbp1},
  normal_constraint_{normal_constraint},
  p_{p},
  stiction_coefficient_{stiction_coefficient},
  friction_coefficient_{friction_coefficient}
{}

void FrictionContactInfo2::solve(float dt, float relaxation) {
    auto get_lambda = [this, dt](const PlaneConstraint& pc) {
        if (pc.active(p_)) {
            float v0 = dot0d(rbp0_.velocity_at_position(p_), pc.plane.normal_);
            float v1 = dot0d(rbp1_.velocity_at_position(p_), pc.plane.normal_);
            float mc0 = rbp0_.effective_mass({.vector = pc.plane.normal_, .position = p_});
            float mc1 = rbp1_.effective_mass({.vector = pc.plane.normal_, .position = p_});
            return - (mc0 * mc1 / (mc0 + mc1)) * (-v0 + v1 + pc.v(p_, dt));
        } else {
            return 0.f;
        }
    };
    FixedArray<float, 2> lambda_total_old = {pcs_[0].lambda_total, pcs_[1].lambda_total};
    FixedArray<float, 2> lambda = {get_lambda(pcs_[0]), get_lambda(pcs_[1])};
    FixedArray<float, 2> lambda_total_new = lambda_total_old + lambda;
    if (float l2 = sum(squared(lambda_total_new)); l2 > squared(max_impulse())) {
        lambda_total_new *= max_impulse() / std::sqrt(l2);
    }
    auto apply_lambda = [this](PlaneConstraint& pc, float lambda) {
        if (pc.active(p_)) {
            pc.lambda_total += lambda;
            rbp0_.integrate_impulse({
                .vector = -pc.plane.normal_ * lambda,
                .position = p_});
            rbp1_.integrate_impulse({
                .vector = pc.plane.normal_ * lambda,
                .position = p_});
        }
    };
    apply_lambda(pcs_[0], lambda_total_new(0) - lambda_total_old(0));
    apply_lambda(pcs_[1], lambda_total_new(1) - lambda_total_old(1));
}

TireContactInfo1::TireContactInfo1(
    const FrictionContactInfo1& fci,
    RigidBody& rb,
    size_t tire_id,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    const PhysicsEngineConfig& cfg)
: fci_{fci},
  rb_{rb},
  P_{rb.consume_tire_surface_power(tire_id)},
  tire_id_{tire_id},
  v3_{v3},
  n3_{n3},
  cfg_{cfg}
{}

void TireContactInfo1::solve(float dt, float relaxation) {
    float force_n1 = -fci_.max_impulse() / dt;
    FixedArray<float, 3> v = -updated_tire_speed(P_, rb_, v3_, n3_, force_n1, cfg_, tire_id_);
    fci_.set_b(v);
    fci_.solve(dt, relaxation);
}

void Mlib::solve_contacts(std::list<std::unique_ptr<ContactInfo>>& cis, float dt) {
    for(size_t i = 0; i < 10; ++i) {
        for(const std::unique_ptr<ContactInfo>& ci : cis) {
            ci->solve(dt, i < 1 ? 0.2 : 1);
        }
    }
}
