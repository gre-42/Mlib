#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geography/Season.hpp>
#include <Mlib/Geography/Sun_Direction.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

// From: https://stackoverflow.com/questions/34857119/how-to-convert-stdchronotime-point-to-string
static std::string serialize_time_point( const std::chrono::system_clock::time_point& time, const std::string& format)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm = *std::gmtime(&tt); //GMT (UTC)
    //std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
    std::stringstream ss;
    ss << std::put_time( &tm, format.c_str() );
    return ss.str();
}

void assert_string_equals(const std::string& a, const std::string& b) {
    if (a != b) {
        THROW_OR_ABORT("Assertion failed: " + a + " != " + b);
    }
}

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

void test_sun_position_day() {
    using namespace std::chrono;
    using namespace std::literals;
    constexpr system_clock::time_point start_time = sys_days{January/1/2042};
    StbImage1 im1{ArrayShape{128, 256}};
    for (size_t itime = 0; itime < im1.shape(1); ++itime) {
        uint64_t duration = (itime * 24 * 60 * 60) / im1.shape(1);
        auto time = start_time + std::chrono::seconds{duration};
        std::cerr << serialize_time_point(time, "%Y-%m-%d %H:%M:%S") << std::endl;
        for (size_t ilat = 0; ilat < im1.shape(0); ++ilat) {
            auto lat = double(ilat) * M_PI / double(im1.shape(0)) - M_PI / 2.;
            auto n = sun_direction(time, lat, 13.4050 * degrees);
            im1(ilat, itime) = (uint8_t)std::clamp(std::round(n(2) * 255.), 0., 255.);
        }
    }
    im1.save_to_file("TestOut/sun_day.png");
}

void test_season() {
    using namespace std::chrono;
    using namespace std::literals;
    constexpr system_clock::time_point start_time = sys_days{January/1/2042} + 12h;
    auto spring = time_of_season(Season::SPRING, start_time, 52.5200 * degrees, 13.4050 * degrees);
    auto summer = time_of_season(Season::SUMMER, start_time, 52.5200 * degrees, 13.4050 * degrees);
    auto autumn = time_of_season(Season::AUTUMN, start_time, 52.5200 * degrees, 13.4050 * degrees);
    auto winter = time_of_season(Season::WINTER, start_time, 52.5200 * degrees, 13.4050 * degrees);
    assert_string_equals(serialize_time_point(spring, "%Y-%m-%d %H:%M:%S"), "2043-03-12 12:00:00");
    assert_string_equals(serialize_time_point(summer, "%Y-%m-%d %H:%M:%S"), "2042-06-10 12:00:00");
    assert_string_equals(serialize_time_point(autumn, "%Y-%m-%d %H:%M:%S"), "2042-09-08 12:00:00");
    assert_string_equals(serialize_time_point(winter, "%Y-%m-%d %H:%M:%S"), "2042-12-12 12:00:00");
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();
    test_sun_position();
    test_sun_position_day();
    test_season();
    return 0;
}
