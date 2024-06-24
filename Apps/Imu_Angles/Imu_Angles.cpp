#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    float ex0 = 0;
    float ey0 = 0;
    float ex1 = 0;
    float ey1 = 0;
    for (unsigned int seed = 1; seed < 100; ++seed) {
        FixedArray<float, 3> angles{uniform_random_array<float>(ArrayShape{3}, seed) - 0.5f};
        FixedArray<float, 3, 3> m = tait_bryan_angles_2_matrix<float>(angles);
        FixedArray<float, 3> g{0.f, 0.f, -9.8f};
        FixedArray<float, 3> a = dot1d(m.T(), g);

        if (seed == 1) {
            lerr() << angles;
            lerr() << a;

            lerr();
            lerr() << -std::atan(a(1) / std::sqrt(squared(a(0)) + squared(a(2)))); // x
            lerr() << std::atan(a(0) / std::sqrt(squared(a(1)) + squared(a(2)))); // y

            lerr();
            lerr() << -std::atan(a(1) / std::sqrt(squared(a(1)) + squared(a(2)))); // x
            lerr() << std::atan(a(0) / std::sqrt(squared(a(0)) + squared(a(2)))); // y

            lerr();
            for (size_t i = 0; i < 3; ++i) {
                for (size_t j = 0; j < 3; ++j) {
                    for (size_t k = 0; k < 3; ++k) {
                        lerr() << i << " " << j << " " << k << "    " << std::atan(a(i) / std::sqrt(squared(a(j)) + squared(a(k))));
                    }
                }
            }
        }
        ex0 = std::max(ex0, std::abs(angles(0) + std::atan(a(1) / std::sqrt(squared(a(0)) + squared(a(2))))));
        ey0 = std::max(ey0, std::abs(angles(1) - std::atan(a(0) / std::sqrt(squared(a(1)) + squared(a(2))))));
        ex1 = std::max(ex1, std::abs(angles(0) + std::atan(a(1) / std::sqrt(squared(a(1)) + squared(a(2))))));
        ey1 = std::max(ey1, std::abs(angles(1) - std::atan(a(0) / std::sqrt(squared(a(0)) + squared(a(2))))));
    }
    lerr() << "ex0 " << ex0 / degrees << "°";
    lerr() << "ey0 " << ey0 / degrees << "°";
    lerr() << "ex1 " << ex1 / degrees << "°";
    lerr() << "ey1 " << ey1 / degrees << "°";
    return 0;
}
