#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Cfd/Lbm/Fluid_Subdomain.hpp>
#include <stdexcept>

using namespace Mlib;

void test_fluid_subdomain() {
    auto frequency = 1.f / 8.f;
    auto omega = 2.f * float(M_PI) * frequency;
    auto center_velocity = FixedArray<float, 2>{0.2f, 0.1f};
    FluidSubdomain<LbmModelD2Q9<float>> cfd{{70u, 20u}};
    for (size_t i = 0; i < 1000; ++i) {
        for (size_t y = 5; y < 10; ++y) {
            cfd.set_velocity_field({40u, y}, std::sin((float)i * omega) * center_velocity);
        }
        //linfo() << "fm0 " << cfd.momentum_field({20u, 7u});
        //linfo() << "fv0 " << cfd.velocity_field({20u, 7u});
        cfd.collide();
        cfd.stream();
        cfd.calculate_macroscopic_variables();
        // linfo() << "fm1 " << cfd.momentum_field({20u, 7u});
        // linfo() << "fv1 " << cfd.velocity_field({20u, 7u});
        cfd.print(lout().ref());
        // lout() << "-----\n";
    }
}

int main(int argc, const char** argv) {
    try {
        test_fluid_subdomain();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
