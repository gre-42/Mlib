#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Points/Feature_Point.hpp>
#include <chrono>
#include <map>
#include <memory>

namespace Mlib { namespace Sfm {

class FeaturePointSequence {
public:
    std::shared_ptr<FeaturePoint> nth_from_end(size_t n);
    std::map<std::chrono::milliseconds, std::shared_ptr<FeaturePoint>> sequence;
};

}}
