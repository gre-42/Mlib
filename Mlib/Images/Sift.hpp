#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib::Sift {

struct KeyPoint {
    OrderableFixedArray<float, 2> pt = uninitialized;
    int octave;
    float size;
    float response;
    std::partial_ordering operator <=> (const KeyPoint& kp) const = default;
};

struct KeyPointWithOrientation {
    KeyPoint kp;
    float angle;
    std::partial_ordering operator <=> (const KeyPointWithOrientation& kp) const = default;
};

struct SiftFeatures {
    std::list<KeyPointWithOrientation> keypoints;
    std::list<Array<float>> descriptors;
};

SiftFeatures computeKeypointsAndDescriptors(
    const Array<float>& image,
    float sigma=1.6f,
    size_t num_intervals=3,
    float assumed_blur=0.5f,
    size_t image_border_width=5);

}
