#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Frames/Forward.hpp>
#include <chrono>
#include <map>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;

namespace Sfm {

class DownSampler {
public:
    DownSampler(const TransformationMatrix<float, 2>& intrinsic_matrix, size_t n);
    void append_image_frame(
        const std::chrono::milliseconds& time,
        const ImageFrame& image_frame);

    std::map<std::chrono::milliseconds, ImageFrame> ds_image_frames_;
    TransformationMatrix<float, 2> ds_intrinsic_matrix_;
private:
    size_t n_;
};

}}
