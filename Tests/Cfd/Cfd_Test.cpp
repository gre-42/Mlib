#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Cfd/Lbm/Fluid_Subdomain.hpp>
#include <stdexcept>

using namespace Mlib;

void test_fluid_subdomain() {
    using T = float;
    auto frequency = (T)1 / 8;
    auto omega = 2.f * T(M_PI) * frequency;
    auto center_velocity = FixedArray<T, 2>{(T)0.1, (T)0.2};
    FluidSubdomain<LbmModelD2Q9<T>> cfd{{70u, 20u}};
    for (size_t i = 0; i < 50; ++i) {
        for (size_t y = 5; y < 10; ++y) {
            cfd.set_velocity_field({35u, y}, std::cos((T)i * omega) * center_velocity);
        }
        cfd.collide();
        cfd.stream();
        cfd.calculate_macroscopic_variables();
        cfd.print(lout().ref());
    }
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();
    try {
        test_fluid_subdomain();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
