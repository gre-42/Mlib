#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Pi.hpp>
#include <cmath>

namespace Mlib {

template <class TData>
void latitude_longitude_2_meters(TData latitude, TData longitude, TData& x, TData& y) {
    TData r0 = 6.371e+6;
    TData r1 = r0 * std::cos(latitude / 180 * M_PI);
    TData circumference0 = r0 * 2 * M_PI;
    TData circumference1 = r1 * 2 * M_PI;
    x = (circumference1 / 360) * longitude;
    y = (circumference0 / 360) * latitude;
}

template <class TData0, class TData1>
void latitude_longitude_2_meters(
    TData0 latitude,
    TData0 longitude,
    TData0 latitude0,
    TData0 longitude0,
    TData1& x,
    TData1& y)
{
    TData0 r0 = 6.371e+6;
    TData0 r1 = r0 * std::cos(latitude0 / 180 * M_PI);
    TData0 circumference0 = r0 * 2 * M_PI;
    TData0 circumference1 = r1 * 2 * M_PI;
    x = (circumference1 / 360) * (longitude - longitude0);
    y = (circumference0 / 360) * (latitude - latitude0);
}

template <class TData>
FixedArray<TData, 3, 3> latitude_longitude_2_meters_matrix(
    TData latitude0,
    TData longitude0)
{
    FixedArray<TData, 3, 3> result;
    TData x;
    TData y;
    latitude_longitude_2_meters<TData, TData>(0, 0, latitude0, longitude0, x, y);
    result(0, 2) = x;
    result(1, 2) = y;
    result(2, 2) = 1;
    latitude_longitude_2_meters<TData, TData>(1, 0, latitude0, longitude0, x, y);
    result(0, 0) = x - result(0, 2);
    result(1, 0) = y - result(1, 2);
    result(2, 0) = 0;
    latitude_longitude_2_meters<TData, TData>(0, 1, latitude0, longitude0, x, y);
    result(0, 1) = x - result(0, 2);
    result(1, 1) = y - result(1, 2);
    result(2, 1) = 0;
    return result;
}

}
