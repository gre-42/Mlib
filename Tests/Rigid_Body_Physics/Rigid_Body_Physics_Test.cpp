#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <fenv.h>

using namespace Mlib;

struct Particle {
    FixedArray<float, 3> x;
    FixedArray<float, 3> v1;
    FixedArray<float, 3> v2b;
    FixedArray<float, 3> v2;
    FixedArray<float, 3, 3> mass;
};

struct PlaneConstraint {
    FixedArray<float, 3> J;
    float b;
    float C(const FixedArray<float, 3>& x) {
        return 0.5f * squared(dot0d(J, x) + b);
    }
};

void test_rigid_body_physics() {
    Particle p{.x = {0, -0.1, 0}, .v1 = {0, -1, 0}, .mass = 5.f * fixed_identity_array<float, 3>()};
    std::list<PlaneConstraint> pcs{{.J = {0, -1, 0}, .b = 0}};
    float h = 1. / 60.;
    float beta = 0.5;
    FixedArray<float, 3> g = {0, -9.8, 0};
    p.v2b = p.v1 + h * g;
    for(size_t i = 0; i < 100; ++i) {
        for(PlaneConstraint& pc: pcs) {
            float lambda = - (dot0d(pc.J, p.v2b) + pc.b + beta / h * pc.C(p.x)) / dot0d(pc.J, solve_symm_1d(p.mass, pc.J));
            p.v2 = p.v2b + solve_symm_1d(p.mass, pc.J * lambda);
            p.x += h * p.v2;
            std::cerr << p.x << " | " << lambda << " | " << p.v2b << " | " << p.v2 << std::endl;
            // p.v2b = p.v2;
        }
    }
}

int main(int argc, const char** argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_rigid_body_physics();
    return 0;
}
