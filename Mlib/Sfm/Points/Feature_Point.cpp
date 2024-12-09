#include "Feature_Point.hpp"

using namespace Mlib::Sfm;

FeaturePoint::FeaturePoint(
    const FixedArray<float, 2>& position,
    const TraceablePatch& traceable_patch,
    const TraceableDescriptor& tracebale_descriptor)
    : position{ position }
    , traceable_patch{ traceable_patch }
    , tracebale_descriptor{ tracebale_descriptor } {}
