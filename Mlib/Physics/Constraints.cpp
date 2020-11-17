#include "Constraints.hpp"
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Pulses.hpp>

using namespace Mlib;

/**
 * From: Erin Catto, Modeling and Solving Constraints
 *       Erin Catto, Fast and Simple Physics using Sequential Impulses
 *       Marijn Tamis, Giuseppe Maggiore, Constraint based physics solver
 *       Marijn Tamis, Sequential Impulse Solver for Rigid Body Dynamics
 */
void ContactInfo1::solve(float dt) {
    if (pc_.active(p_)) {
        float v = dot0d(rbp_.velocity_at_position(p_), pc_.plane.normal_);
        float mc = rbp_.effective_mass({.vector = pc_.plane.normal_, .position = p_});
        float lambda = - mc * (-v + pc_.v(p_, dt));
        lambda = pc_.clamped_lambda(lambda);
        rbp_.integrate_impulse({
            .vector = -pc_.plane.normal_ * lambda,
            .position = p_});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
}

void ContactInfo2::solve(float dt) {
    if (pc_.active(p_)) {
        float v0 = dot0d(rbp0_.velocity_at_position(p_), pc_.plane.normal_);
        float v1 = dot0d(rbp1_.velocity_at_position(p_), pc_.plane.normal_);
        float mc0 = rbp0_.effective_mass({.vector = pc_.plane.normal_, .position = p_});
        float mc1 = rbp1_.effective_mass({.vector = pc_.plane.normal_, .position = p_});
        float lambda = - (mc0 * mc1 / (mc0 + mc1)) * (-v0 + v1 + pc_.v(p_, dt));
        lambda = pc_.clamped_lambda(lambda);
        rbp0_.integrate_impulse({
            .vector = -pc_.plane.normal_ * lambda,
            .position = p_});
        rbp1_.integrate_impulse({
            .vector = pc_.plane.normal_ * lambda,
            .position = p_});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
}

void Mlib::solve_contacts(std::list<std::unique_ptr<ContactInfo>>& cis, float dt) {
    for(size_t i = 0; i < 10; ++i) {
        for(const std::unique_ptr<ContactInfo>& ci : cis) {
            ci->solve(dt);
        }
    }
}
