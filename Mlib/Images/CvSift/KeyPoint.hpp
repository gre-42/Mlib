#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib::cv {

struct KeyPoint {
    OrderableFixedArray<float, 2> pt;
    size_t octave;
    float size;
    float response;
    float angle;
    std::strong_ordering operator <=> (const KeyPoint& kp) const = default;
};

}
