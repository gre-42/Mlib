#include "Constraints.hpp"
#include <Mlib/Geometry/Arbitrary_Orthogonal.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
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

FrictionContactInfo1::FrictionContactInfo1(
    RigidBodyPulses& rbp,
    const PlaneConstraint& normal_constraint,
    const FixedArray<float, 3>& p,
    float stiction_coefficient,
    float friction_coefficient,
    const FixedArray<float, 3>& b)
: rbp_{rbp},
  normal_constraint_{normal_constraint},
  p_{p},
  stiction_coefficient_{stiction_coefficient},
  friction_coefficient_{friction_coefficient}
{
    FixedArray<float, 3> t0 = arbitrary_orthogonal(normal_constraint.plane.normal_);
    t0 /= std::sqrt(sum(squared(t0)));
    FixedArray<float, 3> t1 = cross(t0, normal_constraint.plane.normal_);
    pcs_[0].plane = PlaneNd<float, 3>{t0, p};
    pcs_[1].plane = PlaneNd<float, 3>{t1, p};
    pcs_[0].b = dot0d(t0, b);
    pcs_[1].b = dot0d(t1, b);
}

void FrictionContactInfo1::solve(float dt, float relaxation) {
    for (PlaneConstraint& pc : pcs_) {
        if (pc.active(p_)) {
            pc.lambda_min = std::min(0.f, stiction_coefficient_ * normal_constraint_.lambda_total);
            pc.lambda_max = -std::min(0.f, stiction_coefficient_ * normal_constraint_.lambda_total);
            float v = dot0d(rbp_.velocity_at_position(p_), pc.plane.normal_);
            float mc = rbp_.effective_mass({.vector = pc.plane.normal_, .position = p_});
            float lambda = - mc * (-v + pc.v(p_, dt));
            lambda = pc.clamped_lambda(relaxation * lambda);
            rbp_.integrate_impulse({
                .vector = -pc.plane.normal_ * lambda,
                .position = p_});
        }
    }
}

FrictionContactInfo2::FrictionContactInfo2(
    RigidBodyPulses& rbp0,
    RigidBodyPulses& rbp1,
    const PlaneConstraint& normal_constraint,
    const FixedArray<float, 3>& p,
    float stiction_coefficient,
    float friction_coefficient,
    const FixedArray<float, 3>& b)
: rbp0_{rbp0},
  rbp1_{rbp1},
  normal_constraint_{normal_constraint},
  p_{p},
  stiction_coefficient_{stiction_coefficient},
  friction_coefficient_{friction_coefficient}
{
    FixedArray<float, 3> t0 = arbitrary_orthogonal(normal_constraint.plane.normal_);
    t0 /= std::sqrt(sum(squared(t0)));
    FixedArray<float, 3> t1 = cross(t0, normal_constraint.plane.normal_);
    pcs_[0].plane = PlaneNd<float, 3>{t0, p};
    pcs_[1].plane = PlaneNd<float, 3>{t1, p};
    pcs_[0].b = dot0d(t0, b);
    pcs_[1].b = dot0d(t1, b);
}

void FrictionContactInfo2::solve(float dt, float relaxation) {
    for (PlaneConstraint& pc : pcs_) {
        if (pc.active(p_)) {
            pc.lambda_min = std::min(0.f, stiction_coefficient_ * normal_constraint_.lambda_total);
            pc.lambda_max = -std::min(0.f, stiction_coefficient_ * normal_constraint_.lambda_total);
            float v0 = dot0d(rbp0_.velocity_at_position(p_), pc.plane.normal_);
            float v1 = dot0d(rbp1_.velocity_at_position(p_), pc.plane.normal_);
            float mc0 = rbp0_.effective_mass({.vector = pc.plane.normal_, .position = p_});
            float mc1 = rbp1_.effective_mass({.vector = pc.plane.normal_, .position = p_});
            float lambda = - (mc0 * mc1 / (mc0 + mc1)) * (-v0 + v1 + pc.v(p_, dt));
            lambda = pc.clamped_lambda(relaxation * lambda);
            rbp0_.integrate_impulse({
                .vector = -pc.plane.normal_ * lambda,
                .position = p_});
            rbp1_.integrate_impulse({
                .vector = pc.plane.normal_ * lambda,
                .position = p_});
        }
    }
}

void Mlib::solve_contacts(std::list<std::unique_ptr<ContactInfo>>& cis, float dt) {
    for(size_t i = 0; i < 10; ++i) {
        for(const std::unique_ptr<ContactInfo>& ci : cis) {
            ci->solve(dt, i < 1 ? 0.2 : 1);
        }
    }
}
