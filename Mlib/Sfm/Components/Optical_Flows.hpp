#pragma once
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <chrono>
#include <map>

namespace Mlib::Sfm {

class OpticalFlows {
public:
    OpticalFlows(
        const std::map<std::chrono::milliseconds, ImageFrame>& image_frames,
        const std::string& cache_dir);
    void compute_optical_flow();
    std::map<std::chrono::milliseconds, ImageFrame> optical_flow_frames_;
private:
    const std::map<std::chrono::milliseconds, ImageFrame>& image_frames_;
    std::string cache_dir_;
};

}
