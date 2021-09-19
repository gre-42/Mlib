#pragma once
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <chrono>
#include <map>
#include <ostream>

namespace Mlib::Sfm {

class ImagePipeline {
public:
    virtual ~ImagePipeline() = default;
    virtual void process_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame,
        const CameraFrame* camera_frame = nullptr,
        bool is_last_frame = false,
        bool camera_is_initializer = false) = 0;
    virtual void print_statistics(std::ostream& ostream) = 0;
};

}
