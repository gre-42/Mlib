#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

namespace Mlib {

class ITrailExtender;

struct TrailSource {
    TrailSource(
        std::unique_ptr<ITrailExtender> extender,
        const FixedArray<float, 3> position,
        float minimum_velocity);
    TrailSource(TrailSource&&) = default;
    ~TrailSource();
    std::unique_ptr<ITrailExtender> extender;
    FixedArray<float, 3> position;
    float minimum_velocity;
};

}
