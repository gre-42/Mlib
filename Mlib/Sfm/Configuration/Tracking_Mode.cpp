#include "Tracking_Mode.hpp"
#include <stdexcept>

using namespace Mlib::Sfm;

TrackingMode Mlib::Sfm::tracking_mode_from_string(const std::string& str) {
    if (str == "patches") {
        return TrackingMode::PATCH_NEW_POSITION_IN_BOX;
    } else if (str == "cv_sift") {
        return TrackingMode::CV_SIFT;
    } else if (str == "cv_sift_0") {
        return TrackingMode::CV_SIFT_0;
    } else if (str == "cv_sift_1") {
        return TrackingMode::CV_SIFT_1;
    } else {
        throw std::runtime_error("Unknown tracking mode: \"" + str + '"');
    }
}
