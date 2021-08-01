#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Sfm/Points/Feature_Point.hpp>
#include <memory>

namespace Mlib::Sfm {

class ReconstructedPoint {
public:
    ReconstructedPoint(
        const FixedArray<float, 3>& position,
        float condition_number);
    FixedArray<float, 3> position;
    float condition_number;
};

}
