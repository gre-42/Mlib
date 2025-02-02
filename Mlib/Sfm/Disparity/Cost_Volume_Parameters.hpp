#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;

namespace Sfm {

struct CostVolumeParameters {
    float min_depth = 0.5f;
    float max_depth = 10.f;
    size_t ndepths = 32;
    Array<float> inverse_depths() const;
    float theta_correction_factor() const;
};

}}
