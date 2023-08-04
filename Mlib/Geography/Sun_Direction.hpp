#pragma once
#include <chrono>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class Season;

template <typename TData>
void sun_angles(
    const std::chrono::system_clock::time_point& time,
    TData& latitude,
    TData& longitude);

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
