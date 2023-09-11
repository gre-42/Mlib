#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

class DuplicatePointDetector {
public:
    explicit DuplicatePointDetector(double scale);
    void insert(const FixedArray<double, 3>& p);
private:
    Bvh<double, FixedArray<double, 3>, 3> bvh_;
    double scale_;
};

}
