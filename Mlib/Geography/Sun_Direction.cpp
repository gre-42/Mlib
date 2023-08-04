#include "Sun_Direction.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

// From: https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
template <typename TData>
TData positive_modulo(TData i, TData n) {
    return std::fmod(std::fmod(i, n) + n, n);
}

template <typename TData>
void Mlib::sun_angles(
    const std::chrono::system_clock::time_point& time,
    TData& latitude,
    TData& longitude)
{
    // From: https://en.wikipedia.org/wiki/Position_of_the_Sun
    using namespace std::chrono;
    constexpr system_clock::time_point t0 = sys_days{January/1/2000};
    auto n = std::chrono::duration<double>(time - t0) / (24. * 60. * 60.);
    auto L = 280.460 * degrees + 0.9856474 * degrees * n.count();
    auto g = 357.528 * degrees + 0.9856003 * degrees * n.count();
    L = positive_modulo(L, 2. * M_PI);
    g = positive_modulo(g, 2. * M_PI);
    auto la = L + 1.915 * degrees * std::sin(g) + 0.020 * degrees * std::sin(2 * g);
    auto e = 23.439 * degrees - 0.0000004 * degrees * n.count();
    auto d = std::asin(std::sin(e) * std::sin(la));
    latitude = d;
    longitude = L;
}

template <class TData>
FixedArray<TData, 3> Mlib::sun_direction(
    const std::chrono::system_clock::time_point& time,
    TData latitude,
    TData longitude)
{
    auto R = dot2d(
        rodrigues2(FixedArray<TData, 3>{1., 0., 0.}, latitude),
        rodrigues2(FixedArray<TData, 3>{0., 1., 0.}, longitude));
    TData sun_latitude;
    TData sun_longitude;
    sun_angles(time, sun_latitude, sun_longitude);
    auto sun_dir = FixedArray<TData, 3>{
        std::cos(sun_longitude) * std::cos(sun_latitude),
        std::cos(sun_longitude) * std::sin(sun_latitude),
        std::sin(sun_longitude) * std::cos(sun_latitude)};
    return dot(sun_dir, R);
}

namespace Mlib {

template void sun_angles<double>(const std::chrono::system_clock::time_point&, double& latitude, double& longitude);
template FixedArray<double, 3> sun_direction(const std::chrono::system_clock::time_point&, double latitude, double longitude);

}
