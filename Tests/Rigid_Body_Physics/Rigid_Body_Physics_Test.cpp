#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <fenv.h>

using namespace Mlib;

struct Particle0 {
    FixedArray<float, 3> x;
    FixedArray<float, 3> v1;
    FixedArray<float, 3> v2b;
    FixedArray<float, 3> v2;
    FixedArray<float, 3, 3> mass;
};

struct Particle {
    FixedArray<float, 3> x;
    FixedArray<float, 3> v;
    FixedArray<float, 3, 3> mass;
};

struct PlaneConstraint {
    FixedArray<float, 3> J;
    float b;
    float slop;
    float C(const FixedArray<float, 3>& x) {
        return 0.5f * squared(dot0d(J, x) + b);
    }
    bool active(const FixedArray<float, 3>& x) {
        // std::cerr << dot0d(J, x) + b << std::endl;
        // std::cerr << x << std::endl;
        return dot0d(J, x) + b > slop;
    }
};

void test_rigid_body_physics_0() {
    Particle0 p{.x = {0, -0.1, 0}, .v1 = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
    PlaneConstraint pc{.J = {0, -1, 0}, .b = 0};
    float h = 1. / 60.;
    float beta = 0.5;
    FixedArray<float, 3> g = {0, -9.8, 0};
    p.v2b = p.v1 + h * g;
    for(size_t i = 0; i < 100; ++i) {
        float lambda = - (dot0d(pc.J, p.v2b) + pc.b + beta / h * pc.C(p.x)) / dot0d(pc.J, solve_symm_1d(p.mass, pc.J));
        p.v2 = p.v2b + solve_symm_1d(p.mass, pc.J * lambda);
        // std::cerr << p.x << " | " << lambda << " | " << p.v2b << " | " << p.v2 << std::endl;
        p.v2b = p.v2;
    }
    p.x += h * p.v2;
}

void test_rigid_body_physics_1() {
    Particle p{.x = {0, -0.1, 0}, .v = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
    PlaneConstraint pc{.J = {0, -1, 0}, .b = 0};
    float h = 1. / 60.;
    float beta = 0.5;
    FixedArray<float, 3> g = {0, -9.8, 0};
    p.v += h * g;
    for(size_t i = 0; i < 100; ++i) {
        float lambda = - (dot0d(pc.J, p.v) + pc.b + beta / h * pc.C(p.x)) / dot0d(pc.J, solve_symm_1d(p.mass, pc.J));
        p.v += solve_symm_1d(p.mass, pc.J * lambda);
        // std::cerr << p.x << " | " << lambda << " | " << p.v << std::endl;
    }
    p.x += h * p.v;
}

void test_rigid_body_physics_2() {
    Particle p{.x = {0, 0.2, 0}, .v = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
    PlaneConstraint pc{.J = {0, -1, 0}, .b = 0, .slop = 0.1};
    float h = 1. / 60.;
    float beta = 0.5;
    FixedArray<float, 3> g = {0, -9.8, 0};
    for(size_t i = 0; i < 100; ++i) {
        p.v += h * g;
        if (pc.active(p.x)) {
            for(size_t j = 0; j < 100; ++j) {
                float lambda = - (dot0d(pc.J, p.v) + pc.b + beta / h * pc.C(p.x)) / dot0d(pc.J, solve_symm_1d(p.mass, pc.J));
                p.v += solve_symm_1d(p.mass, pc.J * lambda);
                // p.v2b = p.v2;
            }
        }
        p.x += h * p.v;
        std::cerr << p.x << " | " << p.v << std::endl;
    }
}

// void test_rigid_body_physics_1() {
//     Particle p{.x = {0, -0.1, 0}, .v1 = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
//     float alpha0 = 5 * M_PI / 4;
//     float alpha1 = 7 * M_PI / 4;
//     std::list<PlaneConstraint> pcs{
//         {.J = {std::cos(alpha0), std::sin(alpha0), 0}, .b = 0},
//         {.J = {std::cos(alpha1), std::sin(alpha1), 0}, .b = 0}};
//     float h = 1. / 60.;
//     float beta = 0.5;
//     FixedArray<float, 3> g = {0, -9.8, 0};
//     for(size_t i = 0; i < 10; ++i) {
//         p.v2b = p.v1 + h * g;
//         for(size_t j = 0; j < 100; ++j) {
//             for(PlaneConstraint& pc: pcs) {
//                 // std::cerr << pc.J << std::endl;
//                 float lambda = - (dot0d(pc.J, p.v2b) + pc.b + beta / h * pc.C(p.x)) / dot0d(pc.J, solve_symm_1d(p.mass, pc.J));
//                 p.v2 = p.v2b + solve_symm_1d(p.mass, pc.J * lambda);
//                 p.x += h * p.v2;
//                 // std::cerr << p.x << " | " << lambda << " | " << p.v2b << " | " << p.v2 << std::endl;
//                 // p.v2b = p.v2;
//             }
//         }
//     }
// }

int main(int argc, const char** argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_rigid_body_physics_0();
    test_rigid_body_physics_1();
    test_rigid_body_physics_2();
    return 0;
}
