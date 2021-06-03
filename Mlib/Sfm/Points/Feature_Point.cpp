#include "Feature_Point.hpp"

using namespace Mlib::Sfm;

FeaturePoint::FeaturePoint(
    const FixedArray<float, 2>& position,
    const TraceablePatch& traceable_patch)
:position(position),
 traceable_patch(traceable_patch) {}
