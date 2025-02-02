#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
struct RansacOptions {
    size_t nelems_small;
    size_t ncalls;
    TData inlier_distance_thresh;
    size_t inlier_count_thresh;
    unsigned int seed;
};

}
