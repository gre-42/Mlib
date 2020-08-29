#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Points/Feature_Point.hpp>
#include <memory>

namespace Mlib { namespace Sfm {

class ReconstructedPoint {
public:
    ReconstructedPoint(
        const Array<float>& position,
        float condition_number);
    Array<float> position;
    float condition_number;
};

}}
