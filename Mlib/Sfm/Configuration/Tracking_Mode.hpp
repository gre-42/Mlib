#pragma once
#include <string>

namespace Mlib::Sfm {

enum class TrackingMode {
    PATCH_NEW_POSITION_IN_BOX = 1,
    SIFT = (1 << 1),
    CV_SIFT = (1 << 1) | (1 << 2),
    CV_SIFT_0 = (1 << 1) | (1 << 3),
    CV_SIFT_1 = (1 << 1) | (1 << 4)
};

TrackingMode tracking_mode_from_string(const std::string& str);

inline int operator & (TrackingMode a, TrackingMode b) {
    return (int)a & (int)b;
}

}
