#pragma once
#include <chrono>
#include <compare>

namespace Mlib {
namespace Sfm {

class PointObservation {
public:
    PointObservation(
        const std::chrono::milliseconds& time,
        size_t index);
    std::chrono::milliseconds time;
    size_t index;
    inline std::strong_ordering operator <=> (const PointObservation&) const = default;
};

}
}
