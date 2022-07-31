#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <utility>

namespace Mlib {

struct StreetWayPoint {
    std::pair<double, double> alpha;
    std::pair<FixedArray<double, 3>, FixedArray<double, 3>> edge;
    FixedArray<double, 3> position() const {
        return alpha.first * edge.first + alpha.second * edge.second;
    }
};

}
