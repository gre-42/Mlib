#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <chrono>
#include <map>

namespace Mlib { namespace Sfm {

class DownSampler {
public:
    DownSampler(const Array<float>& intrinsic_matrix, size_t n);
    void append_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame);

    std::map<std::chrono::milliseconds, ImageFrame> ds_image_frames_;
    Array<float> ds_intrinsic_matrix_;
private:
    Array<float> intrinsic_matrix_;
    size_t n_;
};

}}
