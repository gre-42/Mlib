#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TIn>
Array<uint8_t> meters_to_terrarium(const Array<TIn>& meters) {
    if (meters.ndim() != 2) {
        THROW_OR_ABORT("Meters is not a 2D image");
    }
    Array<uint8_t> rgb{ ArrayShape{3, meters.shape(0), meters.shape(1) }};
    // https://www.mapzen.com/blog/elevation/
    Array<TIn> val = meters + TIn(32768);
    for (size_t c = 2; c != SIZE_MAX; --c) {
        auto vc = val - val.applied([](float v){return std::floor(v);});
        rgb[c] = minimum(round(vc * TIn(256)), TIn(255)).template casted<uint8_t>();
        val -= vc;
        val /= 256;
    }
    return rgb;
}

template <class TOut, class TIn>
TOut terrarium_to_meters_pix(const TIn terrarium[3]) {
    return terrarium[0] * TOut(256) + terrarium[1] + terrarium[2] / TOut(256) - TOut(32768);
}

template <class TOut>
Array<TOut> terrarium_to_meters(const Array<uint8_t>& terrarium) {
    if (terrarium.ndim() != 3) {
        THROW_OR_ABORT("Terrarium heightmap does not have 3 dimensions");
    }
    if (terrarium.shape(0) != 3) {
        THROW_OR_ABORT("Terrarium heightmap does not have 3 channels");
    }
    return
        terrarium[0].template casted<TOut>() * TOut(256) +
        terrarium[1].template casted<TOut>() +
        terrarium[2].template casted<TOut>() / TOut(256) - TOut(32768);
}

}
