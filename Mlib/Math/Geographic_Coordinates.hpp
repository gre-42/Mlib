#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <cmath>
#include <string>

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
TransformationMatrix<TData, 2> latitude_longitude_2_meters_mapping(
    TData latitude0,
    TData longitude0)
{
    TransformationMatrix<TData, 2> result;
    TData x;
    TData y;
    latitude_longitude_2_meters<TData, TData>(0, 0, latitude0, longitude0, x, y);
    result.t()(0) = x;
    result.t()(1) = y;
    latitude_longitude_2_meters<TData, TData>(1, 0, latitude0, longitude0, x, y);
    result.R()(0, 0) = x - result.t()(0);
    result.R()(1, 0) = y - result.t()(1);
    latitude_longitude_2_meters<TData, TData>(0, 1, latitude0, longitude0, x, y);
    result.R()(0, 1) = x - result.t()(0);
    result.R()(1, 1) = y - result.t()(1);
    return result;
}

template <class TData>
std::string latitude_to_string(TData latitude) {
    if (latitude == 0) {
        return "0";
    } else if (latitude > 0) {
        return std::to_string(latitude) + " N";
    } else {
        return std::to_string(-latitude) + " S";
    }
}

template <class TData>
std::string longitude_to_string(TData longitude) {
    if (longitude == 0) {
        return "0";
    } else if (longitude > 0) {
        return std::to_string(longitude) + " E";
    } else {
        return std::to_string(-longitude) + " W";
    }
}

template <class TData>
std::string height_to_string(TData height) {
    return std::to_string(height) + " m";
}

}
