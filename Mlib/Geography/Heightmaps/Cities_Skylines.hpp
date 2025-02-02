#pragma once
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
inline Array<uint16_t> meters_to_cities_skylines(const Array<TData>& meters) {
    // https://www.mapzen.com/blog/elevation/
    return round(clipped(meters * TData(64), TData(0), TData(UINT16_MAX))).template casted<uint16_t>();
}

template <class TData>
inline Array<TData> cities_skylines_to_meters(const Array<uint16_t>& cities_skylines) {
    // https://www.mapzen.com/blog/elevation/
    return cities_skylines.casted<TData>() / TData(64);
}

}
