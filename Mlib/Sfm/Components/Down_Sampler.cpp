#include "Down_Sampler.hpp"
#include <Mlib/Images/Resample/Down_Sample_Average.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

DownSampler::DownSampler(const Array<float>& intrinsic_matrix, size_t n)
: n_(n)
{
    ds_intrinsic_matrix_ = intrinsic_matrix.copy();
    ds_intrinsic_matrix_.row_range(0, 2) /= std::pow(2, n);
}

void DownSampler::append_image_frame(
    const std::chrono::milliseconds& time,
    const ImageFrame& image_frame)
{
    ImageFrame f2;
    f2.grayscale = down_sample_average(image_frame.grayscale, n_);
    // f2.mask = down_sample_average(image_frame.mask, n_);
    f2.rgb = multichannel_down_sample_average(image_frame.rgb, n_);
    ds_image_frames_.insert(std::make_pair(time, f2));
}
