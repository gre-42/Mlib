#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>
#include <memory>

namespace Mlib::Sfm {

class FeaturePoint {
public:
    FeaturePoint(
        const FixedArray<float, 2>& position,
        const TraceablePatch& traceable_patch);
    FixedArray<float, 2> position;
    TraceablePatch traceable_patch;
};

}
