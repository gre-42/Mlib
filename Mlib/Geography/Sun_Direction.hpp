#pragma once
#include <chrono>
#include <cstddef>

namespace Mlib {

// From: https://github.com/sczesla/PyAstronomy/blob/master/src/pyasl/asl/eq2hor.py

template <typename TData, size_t... tshape>
class FixedArray;
enum class Season;

template <typename TData>
TData days_since_noon_2000_1_1(const std::chrono::system_clock::time_point& time);

template <typename TData>
void sun_angles(
    double jddays,
    TData& ra,
    TData& dec);

template <class TData>
FixedArray<TData, 3> sun_direction(
    const std::chrono::system_clock::time_point& time,
    TData latitude,
    TData longitude);

template <class TData>
std::chrono::system_clock::time_point time_of_season(
    Season season,
    const std::chrono::system_clock::time_point& start_time,
    TData latitude,
    TData longitude);

}
