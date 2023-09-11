#include "Duplicate_Point_Detector.hpp"
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>

using namespace Mlib;

DuplicatePointDetector::DuplicatePointDetector(double scale)
: bvh_{FixedArray<double, 3>{0.1 * scale, 0.1 * scale, 0.1 * scale}, 17},
  scale_{scale}
{}

void DuplicatePointDetector::insert(const FixedArray<double, 3>& p) {
    const FixedArray<double, 3>* neighbor;
    auto min_dist2 = bvh_.min_distance(
        p,
        1e-3 * scale_,
        [&p](const auto& a){return sum(squared(a - p));},
        &neighbor);
    if (min_dist2 < 1e-3 * scale_) {
        if (any(p != *neighbor)) {
            throw PointException<double, 3>{p, "Detected duplicate points"};
        }
    } else {
        bvh_.insert(AxisAlignedBoundingBox{p}, p);
    }
}
