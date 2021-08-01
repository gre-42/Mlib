#pragma once
#include <Mlib/Sfm/Points/Feature_Point_Sequence.hpp>
#include <map>

namespace Mlib::Sfm {

typedef std::map<size_t, std::shared_ptr<FeaturePointSequence>> FeaturePointFrame;

}
