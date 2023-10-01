#include "Sun_Direction.hpp"
#include <Mlib/Geography/Altitude_Azimuth.hpp>
#include <Mlib/Geography/Idl_Mod.hpp>
#include <Mlib/Geography/Local_Mean_Sidereal_Time.hpp>
#include <Mlib/Geography/Season.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

// constexpr auto jdiff()
// {
//     using namespace std::chrono;
//     using namespace std::chrono_literals;
//     return sys_days{January/1/1970} - (sys_days{November/24/-4713} + 12h);
// }

template <typename TData>
TData Mlib::days_since_noon_2000_1_1(const std::chrono::system_clock::time_point& time) {
    using namespace std::chrono;
    constexpr system_clock::time_point t0 = sys_days{January/1/2000} + 12h;
    return (duration<TData>(time - t0) / (24. * 60. * 60.)).count();
    // return (duration<TData>(time.time_since_epoch()) + jdiff()).count() / (24. * 60. * 60.) - 2451545.;
}

template <typename TData>
void Mlib::sun_angles(
    double jddays,
    TData& ra,
    TData& dec)
{
    // From: https://en.wikipedia.org/wiki/Position_of_the_Sun
    auto L = 280.460 * degrees + 0.9856474 * degrees * jddays;
    auto g = 357.528 * degrees + 0.9856003 * degrees * jddays;
    L = positive_modulo(L, 2. * M_PI);
    g = positive_modulo(g, 2. * M_PI);
    auto la = L + 1.915 * degrees * std::sin(g) + 0.020 * degrees * std::sin(2 * g);
    auto eps = 23.439 * degrees - 0.0000004 * degrees * jddays;
    ra = std::atan2(std::cos(eps) * std::sin(la), std::cos(la));
    dec = std::asin(std::sin(eps) * std::sin(la));
}

// // From: http://star-www.st-and.ac.uk/~fv/webnotes/chapter7.htm
// template <class TData>
// FixedArray<TData, 3> Mlib::sun_direction(
//     const std::chrono::system_clock::time_point& time,
//     TData latitude,
//     TData longitude)
// {
//     using namespace std;
//     auto n = days_since_noon_2000_1_1<TData>(time) - 0.5 + 0.125;
//     auto lst = longitude - (double)remainderl(2. * M_PI * n, 2. * M_PI);
//     TData ra;
//     TData dec;
//     sun_angles(time, ra, dec);
//     auto H = lst - ra;
//     auto altitude = asin(sin(dec) * sin(latitude) + cos(dec) * cos(latitude) * cos(H));
//     auto sin_A = -sin(H) * cos(dec) / cos(altitude);
//     auto cos_A = (sin(dec) - sin(latitude) * sin(altitude)) / (cos(latitude) * cos(altitude));
//     auto azimuth = atan2(sin_A, cos_A);
//     return -FixedArray<TData, 3>{
//         sin(azimuth),
//         cos(azimuth),
//         sin(altitude)};
// }

template <class TData>
FixedArray<TData, 3> Mlib::sun_direction(
    const std::chrono::system_clock::time_point& time,
    TData latitude,
    TData longitude)
{
    auto jddays = days_since_noon_2000_1_1<TData>(time);
    TData sun_ra;
    TData sun_dec;
    sun_angles(jddays, sun_ra, sun_dec);
    auto lmst = ct2lst(jddays, longitude) * 15 * degrees;

    // Find hour angle (in DEGREES)
    auto ha = lmst - sun_ra;
    if (ha < 0) {
        ha += 360. * degrees;
    }

    ha = idl_mod(ha, 360. * degrees);

    double alt;
    double az;
    hadec2altaz(ha, sun_dec, latitude, alt, az);

    return -FixedArray<TData, 3>{
        std::sin(az),
        std::cos(az),
        std::sin(alt)};
}

template <class TData>
std::chrono::system_clock::time_point Mlib::time_of_season(
    Season season,
    const std::chrono::system_clock::time_point& start_time,
    TData latitude,
    TData longitude)
{
    if (season == Season::SPRING) {
        return time_of_season(Season::WINTER, start_time, latitude, longitude) + std::chrono::seconds{3 * 30 * 24 * 60 * 60};
    }
    if (season == Season::AUTUMN) {
        return time_of_season(Season::SUMMER, start_time, latitude, longitude) + std::chrono::seconds{3 * 30 * 24 * 60 * 60};
    }
    if ((season == Season::SUMMER) ||
        (season == Season::WINTER))
    {
        TData best_brightness = -INFINITY;
        auto result = std::chrono::system_clock::time_point();
        for (size_t i = 0; i < 365; ++i) {
            auto time = start_time + std::chrono::days{i};
            auto brightness = -sun_direction(time, latitude, longitude)(2);
            if (season == Season::WINTER) {
                brightness = -brightness;
            }
            if (brightness > best_brightness) {
                result = time;
                best_brightness = brightness;
            }
        }
        return result;
    }
    THROW_OR_ABORT("Unknown season");
}

namespace Mlib {

template double days_since_noon_2000_1_1<double>(const std::chrono::system_clock::time_point&);
template void sun_angles(double jddays, double& latitude, double& longitude);
template FixedArray<double, 3> sun_direction(const std::chrono::system_clock::time_point&, double latitude, double longitude);
template std::chrono::system_clock::time_point time_of_season<double>(
    Season, const std::chrono::system_clock::time_point& start_time, double latitude, double longitude);

}
