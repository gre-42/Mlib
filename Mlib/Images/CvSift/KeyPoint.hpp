#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib::ocv {

struct KeyPoint {
    OrderableFixedArray<float, 2> pt = uninitialized;
    int octave;
    float size;
    float response;
    float angle;
    std::partial_ordering operator <=> (const KeyPoint& kp) const = default;
};

}
