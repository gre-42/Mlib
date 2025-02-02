#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Points/Feature_Point.hpp>
#include <chrono>
#include <map>
#include <memory>

namespace Mlib::Sfm {

class FeaturePointSequence {
public:
    std::map<std::chrono::milliseconds, std::shared_ptr<FeaturePoint>> sequence;
};

}
