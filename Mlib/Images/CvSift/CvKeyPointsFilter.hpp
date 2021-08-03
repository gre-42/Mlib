#pragma once
#include <cstdint>
#include <vector>

namespace Mlib::cv {
    struct KeyPoint;
    template <class TData>
    class Mat;
};

namespace Mlib::cv::KeyPointsFilter {

void removeDuplicated( std::vector<KeyPoint>& keypoints );
void runByPixelsMask( std::vector<KeyPoint>& keypoints, const Mat<uint8_t>& mask );
void retainBest(std::vector<KeyPoint>& keypoints, int n_points);

}
