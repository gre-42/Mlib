#include "Resize.hpp"
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Get_Target_Shape.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <stb/stb_image_resize2.h>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>
#include <stdexcept>

using namespace Mlib;

Array<float> Mlib::resized_multichannel(
    const Array<float>& source,
    const ArrayShape& target_shape,
    TargetShapeMode target_shape_mode,
    FilterExtension fc,
    float sigma)
{
    if (source.ndim() != 3) {
        throw std::runtime_error("Image does not have 3 dimensions");
    }
    if (target_shape.ndim() != source.ndim() - 1) {
        throw std::runtime_error("Target shape mismatch");
    }
    auto xtarget_shape = get_target_shape(source.shape().erased_first(), target_shape, target_shape_mode);
    if (any(xtarget_shape == 0)) {
        return zeros<float>(ArrayShape{1}.concatenated(xtarget_shape));
    }
     Array<float> smoothed;
    if (sigma != 0.f) {
        smoothed = source.copy();
        for (size_t h = 0; h < source.shape(0); ++h) {
            for (size_t d = 0; d < xtarget_shape.ndim(); ++d) {
                float fac = integral_to_float<float>(source.shape(1u + d)) / integral_to_float<float>(xtarget_shape(d));
                if (fac > 1) {
                    smoothed[h] = gaussian_filter_1d_NWE(smoothed[h], sigma * fac, d, NAN, 2.f, fc);
                }
            }
        }
    } else {
        smoothed = source;
    }
    auto stb_smoothed = StbInfo<float>{
        integral_cast<int>(source.shape(2)),
        integral_cast<int>(source.shape(1)),
        integral_cast<int>(source.shape(0))};
    array_2_stb_image(smoothed, stb_smoothed.data());
    auto result = StbInfo<float>{
        integral_cast<int>(xtarget_shape(1)),
        integral_cast<int>(xtarget_shape(0)),
        integral_cast<int>(source.shape(0))};
    {
        TemporarilyIgnoreFloatingPointExeptions ignore_except;
        if (!stbir_resize_float_linear(
            stb_smoothed.data(),
            stb_smoothed.width,
            stb_smoothed.height,
            0,
            result.data(),
            integral_cast<int>(xtarget_shape(1)),
            integral_cast<int>(xtarget_shape(0)),
            0,
            (stbir_pixel_layout)integral_cast<int>(source.shape(0))))
        {
            throw std::runtime_error("could not resize image");
        }
    }
    return stb_image_2_array(result);
}

Array<float> Mlib::resized_singlechannel(
    const Array<float>& source,
    const ArrayShape& target_shape,
    TargetShapeMode target_shape_mode,
    FilterExtension fc,
    float sigma)
{
    return resized_multichannel(source.reshaped(ArrayShape{1}.concatenated(source.shape())), target_shape, target_shape_mode, fc)[0];
}

Array<float> Mlib::enlarged_multichannel_2d(
    const Array<float>& image,
    size_t factor)
{
    if (image.ndim() != 3) {
        throw std::runtime_error("Unexpected image dimensions");
    }
    return resized_multichannel(image, image.shape().erased_first() * factor, TargetShapeMode::DEST, FilterExtension::NONE, 0.f);
}

Array<float> Mlib::compressed_multichannel_2d(
    const Array<float>& image,
    size_t factor,
    FilterExtension fc)
{
    if (image.ndim() != 3) {
        throw std::runtime_error("Unexpected image dimensions");
    }
    return resized_multichannel(image, image.shape().erased_first() / factor, TargetShapeMode::DEST, fc, 0.f);
}

