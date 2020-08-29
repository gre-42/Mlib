#pragma once
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <chrono>
#include <map>
#include <ostream>

namespace Mlib { namespace Sfm {

class ImagePipeline {
public:
    virtual void process_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame,
        const CameraFrame* camera_frame = nullptr) = 0;
    virtual void print_statistics(std::ostream& ostream) = 0;
};

}}
