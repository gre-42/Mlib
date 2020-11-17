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
void ContactInfo1::solve(float dt, float* lambda_total) const {
    float lt = 0;
    if (pc.active(p)) {
        float v = dot0d(rbp.velocity_at_position(p), pc.plane.normal_);
        float mc = rbp.effective_mass({.vector = pc.plane.normal_, .position = p});
        float lambda = - mc * (-v + pc.v(p, dt));
        lambda = std::clamp(lt + lambda, pc.lambda_min, pc.lambda_max) - lt;
        lt += lambda;
        rbp.integrate_impulse({
            .vector = -pc.plane.normal_ * lambda,
            .position = p});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
    if (lambda_total != nullptr) {
        *lambda_total = lt;
    }
}

void ContactInfo2::solve(float dt, float* lambda_total) const {
    float lt = 0;
    if (pc.active(p)) {
        float v0 = dot0d(rbp0.velocity_at_position(p), pc.plane.normal_);
        float v1 = dot0d(rbp1.velocity_at_position(p), pc.plane.normal_);
        float mc0 = rbp0.effective_mass({.vector = pc.plane.normal_, .position = p});
        float mc1 = rbp1.effective_mass({.vector = pc.plane.normal_, .position = p});
        float lambda = - (mc0 * mc1 / (mc0 + mc1)) * (-v0 + v1 + pc.v(p, dt));
        lambda = std::clamp(lt + lambda, pc.lambda_min, pc.lambda_max) - lt;
        lt += lambda;
        rbp0.integrate_impulse({
            .vector = -pc.plane.normal_ * lambda,
            .position = p});
        rbp1.integrate_impulse({
            .vector = pc.plane.normal_ * lambda,
            .position = p});
        // std::cerr << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x) << std::endl;
    }
    if (lambda_total != nullptr) {
        *lambda_total = lt;
    }
}

void Mlib::solve_contacts(std::list<std::unique_ptr<ContactInfo>>& cis, float dt) {
    for(size_t i = 0; i < 10; ++i) {
        for(const std::unique_ptr<ContactInfo>& ci : cis) {
            ci->solve(dt);
        }
    }
}
