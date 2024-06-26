#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Collision/Resolve/Constraints.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>

using namespace Mlib;

struct Particle0 {
    FixedArray<double, 3> x;
    FixedArray<float, 3> v1;
    FixedArray<float, 3> v2b;
    FixedArray<float, 3> v2;
    FixedArray<float, 3, 3> mass;
};

struct Particle {
    FixedArray<double, 3> x;
    FixedArray<float, 3> v;
    FixedArray<float, 3, 3> mass;
};

void test_rigid_body_physics_particle0() {
    Particle0 p{.x = {0, -0.1, 0}, .v1 = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
    PlaneInequalityConstraint pc{.normal_impulse{.normal = {0, 1, 0}}, .intercept = 0, .b = 0};
    float h = 1.f / 60.f;
    float beta = 0.5f;
    FixedArray<float, 3> g = {0, -9.8f, 0};
    p.v2b = p.v1 + h * g;
    for (size_t i = 0; i < 100; ++i) {
        FixedArray<float, 3> J = -pc.normal_impulse.normal.casted<float>();
        float lambda = - (dot0d(J, p.v2b) + pc.b + beta / h * pc.C(p.x)) / dot0d(J, solve_symm_1d(p.mass, J).value());
        p.v2 = p.v2b + solve_symm_1d(p.mass, J * lambda).value();
        // lerr() << p.x << " | " << lambda << " | " << p.v2b << " | " << p.v2;
        p.v2b = p.v2;
    }
    p.x += (h * p.v2).casted<double>();
}

void test_rigid_body_physics_particle() {
    Particle p{.x = {0, -0.1, 0}, .v = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
    PlaneInequalityConstraint pc{.normal_impulse{.normal = {0, 1, 0}}, .intercept = 0, .b = 0, .always_active = false};
    float h = 1. / 60.;
    float beta = 0.5;
    FixedArray<float, 3> g = {0, -9.8, 0};
    p.v += h * g;
    for (size_t i = 0; i < 100; ++i) {
        FixedArray<float, 3> J = -pc.normal_impulse.normal.casted<float>();
        float lambda = - (dot0d(J, p.v) + pc.b + beta / h * pc.C(p.x)) / dot0d(J, solve_symm_1d(p.mass, J).value());
        p.v += solve_symm_1d(p.mass, J * lambda).value();
        // lerr() << p.x << " | " << lambda << " | " << p.v;
    }
    p.x += (h * p.v).casted<double>();
}

void test_rigid_body_physics_timestep() {
    Particle p{.x = {0, 0.2, 0}, .v = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
    PlaneInequalityConstraint pc{.normal_impulse{.normal = {0, 1, 0}}, .intercept = 0, .b = 0, .slop = 0.01, .always_active = false};
    float h = 1. / 60.;
    float beta = 0.5;
    float beta2 = 0.2;
    FixedArray<float, 3> g = {0, -9.8, 0};
    std::list<float> xs;
    std::list<float> ys;
    for (size_t i = 0; i < 100; ++i) {
        p.v += h * g;
        if (pc.active(p.x)) {
            for (size_t j = 0; j < 100; ++j) {
                FixedArray<float, 3> J = -pc.normal_impulse.normal.casted<float>();
                float lambda = - (dot0d(J, p.v) + pc.b + 1.f / h * (beta * pc.C(p.x) - beta2 * pc.bias(p.x))) / dot0d(J, solve_symm_1d(p.mass, J).value());
                p.v += solve_symm_1d(p.mass, J * lambda).value();
            }
        }
        p.x += (h * p.v).casted<double>();
        // lerr() << p.x << " | " << p.v << " | " << pc.active(p.x) << " | " << pc.overlap(p.x) << " | " << pc.bias(p.x);
        xs.push_back(i);
        ys.push_back(p.x(1));
    }
    if (false) {
        std::ofstream f{"/tmp/plot.svg"};
        Svg svg{f, 600, 500};
        svg.plot(xs, ys);
        svg.finish();
    }
}

/**
 * From: Erin Catto, Modeling and Solving Constraints
 *       Erin Catto, Fast and Simple Physics using Sequential Impulses
 *       Marijn Tamis, Giuseppe Maggiore, Constraint based physics solver
 *       Marijn Tamis, Sequential Impulse Solver for Rigid Body Dynamics
 */
void test_rigid_body_physics_rbi() {
    RigidBodyPulses rbp = rigid_cuboid_pulses(10, {1, 2, 3}, {0, 0, 0});
    rbp.set_pose(fixed_identity_array<float, 3>(), {0, 0.2, 0});
    rbp.v_(1) = -1;
    PlaneInequalityConstraint pc{
        .normal_impulse{.normal = {0, 1, 0}},
        .intercept = 0,
        .slop = 0.01,
        .always_active = false};
    float h = 1. / 60.;
    float beta = 0.5;
    float beta2 = 0.2;
    FixedArray<float, 3> g = {0, -9.8, 0};
    // std::list<float> xs;
    // std::list<float> ys;
    for (size_t i = 0; i < 100; ++i) {
        rbp.v_ += h * g;
        FixedArray<double, 3> p = rbp.abs_position() - FixedArray<double, 3>{0, 0.1, 0};
        // lerr() << x;
        // lerr() << rbp.rotation_;
        // lerr() << rbp.abs_com_;
        // lerr() << rbp.com_;
        if (pc.active(p)) {
            for (size_t j = 0; j < 100; ++j) {
                float v = dot0d(rbp.velocity_at_position(p), pc.normal_impulse.normal.casted<float>());
                float mc = rbp.effective_mass({.vector = pc.normal_impulse.normal.casted<float>(), .position = p});
                float lambda = - mc * (-v + pc.b + 1.f / h * (beta * pc.C(p) - beta2 * pc.bias(p)));
                rbp.v_ -= pc.normal_impulse.normal.casted<float>() / rbp.mass_ * lambda;
                rbp.w_ -= rbp.solve_abs_I(cross(p - rbp.abs_com_, pc.normal_impulse.normal).casted<float>()) * lambda;
                // lerr() << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x);
            }
        }
        rbp.advance_time(h);
        // lerr() << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x);
        // xs.push_back(i);
        // ys.push_back(p.x(1));
    }
    // std::ofstream f{"/tmp/plot.svg"};
    // Svg svg{f, 600, 500};
    // svg.plot(xs, ys);
    // svg.finish();
}

void test_rigid_body_physics_rbi_multiple() {
    RigidBodyPulses rbp = rigid_cuboid_pulses(10, {1, 2, 3}, {0, 0, 0});
    rbp.set_pose(fixed_identity_array<float, 3>(), {0, 0.2, 0});
    rbp.v_(1) = -1;
    BoundedPlaneInequalityConstraint pc{
        .constraint{
            .normal_impulse{.normal={0, 1, 0}},
            .intercept = 0,
            .slop = 0.01,
            .always_active = false}};
    float h = 1. / 60.;
    FixedArray<float, 3> g = {0, -9.8, 0};
    std::list<float> xs;
    std::list<float> ys;
    for (size_t i = 0; i < 100; ++i) {
        rbp.v_ += h * g;
        std::list<std::unique_ptr<ContactInfo>> cis;
        cis.push_back(
            std::unique_ptr<ContactInfo>(
                new NormalContactInfo1{
                    rbp,
                    pc,
                    rbp.transform_to_world_coordinates({-0.2, -0.1, 0})}));
        cis.push_back(
            std::unique_ptr<ContactInfo>(
                new NormalContactInfo1{
                    rbp,
                    pc,
                    rbp.transform_to_world_coordinates({0.2, -0.1, 0})}));
        // lerr() << rbp.abs_position();
        // lerr() << rbp.rotation_;
        // lerr() << rbp.abs_com_;
        // lerr() << rbp.com_;
        solve_contacts(cis, h);
        rbp.advance_time(h);
        // lerr() << rbp.abs_position() << " | " << rbp.v_ << " | " << pc.active(x) << " | " << pc.overlap(x) << " | " << pc.bias(x);
        xs.push_back(i);
        // ys.push_back(rbp.abs_position()(1));
        ys.push_back(rbp.w_(2));
    }
    if (false) {
        std::ofstream f{"/tmp/plot.svg"};
        Svg svg{f, 600, 500};
        svg.plot(xs, ys);
        svg.finish();
        f.flush();
        if (f.fail()) {
            throw std::runtime_error("Could not write plot file");
        }

    }
}

// void test_rigid_body_physics_1() {
//     Particle p{.x = {0, -0.1, 0}, .v1 = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
//     float alpha0 = 5 * M_PI / 4;
//     float alpha1 = 7 * M_PI / 4;
//     std::list<PlaneInequalityConstraint> pcs{
//         {.J = {std::cos(alpha0), std::sin(alpha0), 0}, .b = 0},
//         {.J = {std::cos(alpha1), std::sin(alpha1), 0}, .b = 0}};
//     float h = 1. / 60.;
//     float beta = 0.5;
//     FixedArray<float, 3> g = {0, -9.8, 0};
//     for (size_t i = 0; i < 10; ++i) {
//         p.v2b = p.v1 + h * g;
//         for (size_t j = 0; j < 100; ++j) {
//             for (PlaneInequalityConstraint& pc: pcs) {
//                 // lerr() << pc.J;
//                 float lambda = - (dot0d(pc.J, p.v2b) + pc.b + beta / h * pc.C(p.x)) / dot0d(pc.J, solve_symm_1d(p.mass, pc.J));
//                 p.v2 = p.v2b + solve_symm_1d(p.mass, pc.J * lambda);
//                 p.x += h * p.v2;
//                 // lerr() << p.x << " | " << lambda << " | " << p.v2b << " | " << p.v2;
//                 // p.v2b = p.v2;
//             }
//         }
//     }
// }


int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    test_rigid_body_physics_particle0();
    test_rigid_body_physics_particle();
    test_rigid_body_physics_timestep();
    test_rigid_body_physics_rbi();
    test_rigid_body_physics_rbi_multiple();
    return 0;
}
