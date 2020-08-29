#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Pi.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    float ex0 = 0;
    float ey0 = 0;
    float ex1 = 0;
    float ey1 = 0;
    for(size_t seed = 1; seed < 100; ++seed) {
        FixedArray<float, 3> angles{random_array4<float>(ArrayShape{3}, seed) - 0.5f};
        FixedArray<float, 3, 3> m = tait_bryan_angles_2_matrix<float>(angles);
        FixedArray<float, 3> g{0, 0, -9.8};
        FixedArray<float, 3> a = dot(m.T(), g);

        if (seed == 1) {
            std::cerr << angles << std::endl;
            std::cerr << a << std::endl;

            std::cerr << std::endl;
            std::cerr << -std::atan(a(1) / std::sqrt(squared(a(0)) + squared(a(2)))) << std::endl; // x
            std::cerr << std::atan(a(0) / std::sqrt(squared(a(1)) + squared(a(2)))) << std::endl; // y

            std::cerr << std::endl;
            std::cerr << -std::atan(a(1) / std::sqrt(squared(a(1)) + squared(a(2)))) << std::endl; // x
            std::cerr << std::atan(a(0) / std::sqrt(squared(a(0)) + squared(a(2)))) << std::endl; // y

            std::cerr << std::endl;
            for(size_t i = 0; i < 3; ++i) {
                for(size_t j = 0; j < 3; ++j) {
                    for(size_t k = 0; k < 3; ++k) {
                        std::cerr << i << " " << j << " " << k << "    " << std::atan(a(i) / std::sqrt(squared(a(j)) + squared(a(k)))) << std::endl;
                    }
                }
            }
        }
        ex0 = std::max(ex0, std::abs(angles(0) + std::atan(a(1) / std::sqrt(squared(a(0)) + squared(a(2))))));
        ey0 = std::max(ey0, std::abs(angles(1) - std::atan(a(0) / std::sqrt(squared(a(1)) + squared(a(2))))));
        ex1 = std::max(ex1, std::abs(angles(0) + std::atan(a(1) / std::sqrt(squared(a(1)) + squared(a(2))))));
        ey1 = std::max(ey1, std::abs(angles(1) - std::atan(a(0) / std::sqrt(squared(a(0)) + squared(a(2))))));
    }
    std::cerr << "ex0 " << ex0 * 180 / M_PI << "°" << std::endl;
    std::cerr << "ey0 " << ey0 * 180 / M_PI << "°" << std::endl;
    std::cerr << "ex1 " << ex1 * 180 / M_PI << "°" << std::endl;
    std::cerr << "ey1 " << ey1 * 180 / M_PI << "°" << std::endl;
    return 0;
}
