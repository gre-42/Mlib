#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geography/Season.hpp>
#include <Mlib/Geography/Sun_Direction.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Physics/Units.hpp>

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

static std::string serializeTimePoint( const std::chrono::system_clock::time_point& time, const std::string& format)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm = *std::gmtime(&tt); //GMT (UTC)
    //std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
    std::stringstream ss;
    ss << std::put_time( &tm, format.c_str() );
    return ss.str();
}

void test_season() {
    using namespace std::chrono;
    constexpr system_clock::time_point start_time = sys_days{January/1/2042};
    auto spring = time_of_season(Season::SPRING, start_time, 52.5200 * degrees, 13.4050 * degrees);
    auto summer = time_of_season(Season::SUMMER, start_time, 52.5200 * degrees, 13.4050 * degrees);
    auto autumn = time_of_season(Season::AUTUMN, start_time, 52.5200 * degrees, 13.4050 * degrees);
    auto winter = time_of_season(Season::WINTER, start_time, 52.5200 * degrees, 13.4050 * degrees);
    assert_true(serializeTimePoint(spring, "%Y-%m-%d %H:%M:%S") == "2043-03-12 00:00:00");
    assert_true(serializeTimePoint(summer, "%Y-%m-%d %H:%M:%S") == "2042-06-11 00:00:00");
    assert_true(serializeTimePoint(autumn, "%Y-%m-%d %H:%M:%S") == "2042-09-09 00:00:00");
    assert_true(serializeTimePoint(winter, "%Y-%m-%d %H:%M:%S") == "2042-12-12 00:00:00");
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();
    test_season();

    test_sun_position();
    return 0;
}
