#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <chrono>
#include <map>

namespace Mlib { namespace Sfm {

class DownSampler {
public:
    DownSampler(const FixedArray<float, 3, 3>& intrinsic_matrix, size_t n);
    void append_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame);

    std::map<std::chrono::milliseconds, ImageFrame> ds_image_frames_;
    FixedArray<float, 3, 3> ds_intrinsic_matrix_;
private:
    size_t n_;
};

}}
