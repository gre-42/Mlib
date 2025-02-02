#include "Save_Movie.hpp"
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void SaveMovie::save(
    const std::string& file_prefix,
    const std::string& file_suffix,
    size_t width,
    size_t height)
{
    // VectorialPixels<float, 3> vp{ArrayShape{size_t(height), size_t(width)}};
    // CHK(glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_RGB, GL_FLOAT, vp->flat_begin()));
    // if (normalize) {
    //     draw_nan_masked_rgb(reverted_axis(vp.to_array(), 1), 0.f, 1.f).save_to_file(file_prefix + std::to_string(index_) + file_suffix + ".png");
    // } else {
    //     PpmImage::from_float_rgb(reverted_axis(vp.to_array(), 1)).save_to_file(file_prefix + std::to_string(index_) + file_suffix + ".ppm");
    // }
    StbImage3 im3{ FixedArray<size_t, 2>{ height, width} };
    CHK(glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_RGB, GL_UNSIGNED_BYTE, im3.flat_begin()));
    im3.save_to_file(file_prefix + std::to_string(index_) + file_suffix + ".png");
    ++index_;
}
