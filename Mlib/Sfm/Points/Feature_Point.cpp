#include "Feature_Point.hpp"

using namespace Mlib::Sfm;

FeaturePoint::FeaturePoint(
    const Array<float>& position,
    const TraceablePatch& traceable_patch)
:position(position),
 traceable_patch(traceable_patch) {}
