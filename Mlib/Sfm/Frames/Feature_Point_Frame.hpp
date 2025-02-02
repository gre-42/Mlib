#pragma once
#include <Mlib/Sfm/Points/Feature_Point_Sequence.hpp>
#include <map>

namespace Mlib::Sfm {

enum class TrackingMode;

struct FeaturePointFrame {
    typedef std::map<size_t, std::shared_ptr<FeaturePointSequence>> TrackedPoints;
    TrackedPoints tracked_points;
    Array<FixedArray<float, 2>> keypoints;
    Array<float> descriptors;
    TrackingMode tracking_mode;
};

}
