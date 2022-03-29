#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <utility>

namespace Mlib {

struct StreetWayPoint {
    std::pair<float, float> alpha;
    std::pair<FixedArray<float, 3>, FixedArray<float, 3>> edge;
    FixedArray<float, 3> position() const {
        return alpha.first * edge.first + alpha.second * edge.second;
    }
};

}
