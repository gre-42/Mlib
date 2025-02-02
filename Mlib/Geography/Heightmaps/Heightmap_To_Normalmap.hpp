#pragma once
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

// From: https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
template <class T>
Array<T> heightmap_to_normalmap(const Array<T>& heightmap, const T& d) {
    if (heightmap.ndim() != 2) {
        THROW_OR_ABORT("Heightmap is not 2D");
    }
    auto normalmap = Array<T>(ArrayShape{3, heightmap.shape(0), heightmap.shape(1)});
    normalmap.row_range(0, 2) = central_gradient_filter(heightmap) * (T(1) / (2 * d));
    normalmap[2] = T(1);
    auto fac = T(1) / sqrt(sum(squared(normalmap), 0));
    for (size_t i = 0; i < 3; ++i) {
        normalmap[i] *= fac;
    }
    return normalmap;
}

}
