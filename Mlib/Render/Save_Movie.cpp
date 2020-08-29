#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Save_Movie.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void SaveMovie::save(
    const std::string& file_prefix,
    const std::string& file_suffix,
    size_t width,
    size_t height,
    bool normalize)
{
    VectorialPixels<float, 3> vp{ArrayShape{size_t(height), size_t(width)}};
    CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
    if (normalize) {
        draw_nan_masked_rgb(reverted_axis(vp.to_array(), 1), 0, 0).save_to_file(file_prefix + std::to_string(index_) + file_suffix + ".pgm");
    } else {
        PpmImage::from_float_rgb(reverted_axis(vp.to_array(), 1)).save_to_file(file_prefix + std::to_string(index_) + file_suffix + ".pgm");
    }
    ++index_;
}
