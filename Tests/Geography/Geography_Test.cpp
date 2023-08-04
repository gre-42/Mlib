#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geography/Sun_Direction.hpp>
#include <Mlib/Images/StbImage1.hpp>

using namespace Mlib;

void test_sun_position() {
    using namespace std::chrono;
    constexpr system_clock::time_point time = sys_days{February/4/2042};
    StbImage1 im1{ArrayShape{256, 512}};
    for (size_t ilat = 0; ilat < im1.shape(0); ++ilat) {
        for (size_t ilon = 0; ilon < im1.shape(1); ++ilon) {
            auto lat = double(ilat) * M_PI / double(im1.shape(0)) - M_PI / 2.;
            auto lon = double(ilon) * 2. * M_PI / double(im1.shape(1));
            auto n = sun_direction(time, lat, lon);
            im1(ilat, ilon) = (uint8_t)std::clamp(std::round(n(2) * 255.), 0., 255.);
        }
    }
    im1.save_to_file("TestOut/sun.png");
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();

    test_sun_position();
    return 0;
}
