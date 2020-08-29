#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>
#include <memory>

namespace Mlib { namespace Sfm {

class FeaturePoint {
public:
    FeaturePoint(
        const Array<float>& position,
        const TraceablePatch& traceable_patch);
    Array<float> position;
    TraceablePatch traceable_patch;
};

}}
