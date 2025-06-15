#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Cfd/Lbm/Fluid_Subdomain.hpp>
#include <Mlib/Time/Elapsed_Guard.hpp>
#include <Mlib/Time/Sleep.hpp>
#include <stdexcept>

using namespace Mlib;

template <class TModel>
void test_fluid_subdomain() {
    using T = TModel::type;
    auto frequency = (T)1 / 8;
    auto omega = 2.f * T(M_PI) * frequency;
    auto center_velocity = FixedArray<T, 2>{(T)0.2, (T)0.2};
    FluidSubdomain<TModel> cfd{{70u, 20u}};
    for (size_t i = 0; i < 200; ++i) {
        auto x = std::clamp<size_t>(float_to_integral<size_t>(35 + std::round(i / 10.0)), 0, cfd.size(0) - 1);
        for (size_t y = 5; y < 10; ++y) {
            cfd.set_velocity_field({x, y}, std::cos((T)i * omega) * center_velocity);
        }
        cfd.iterate();
        cfd.print_momentum(lout().ref());
        cfd.print_density(lout().ref());
        sleep_for(std::chrono::milliseconds(15));
    }
    {
        ElapsedGuard eg;
        for (size_t i = 0; i < 1000; ++i) {
            for (size_t y = 5; y < 10; ++y) {
                cfd.set_velocity_field({35u, y}, std::cos((T)i * omega) * center_velocity);
            }
            cfd.iterate();
        }
    }
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();
    try {
        test_fluid_subdomain<LbmModelD2Q9<float>>();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
