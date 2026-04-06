#include "Brightness_Image.hpp"
#include <Mlib/Images/Extrapolate_Rgba_Colors.hpp>
#include <Mlib/Images/Get_Target_Shape.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Transform/Resize.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

static const float SCALE = 1.f;
static const float OFFSET = 1e-6f;

BrightnessImage::BrightnessImage(
    const Array<float>& color,
    const Array<float>& brightness_and_alpha)
    : color(color)
    , brightness_and_alpha(brightness_and_alpha)
{}
    
BrightnessImage::BrightnessImage(
        const Array<float>& source,
        size_t color_width,
        size_t color_height,
        size_t structure_width,
        size_t structure_height,
        TargetShapeMode target_shape_mode)
{
    if (source.ndim() != 3) {
        throw std::runtime_error("Image does not have 3 dimensions");
    }
    auto xp3 = (source.shape(0) == 3) ? source : extrapolate_rgba_colors(source, 3.f, 99999).row_range(0, 3);
    auto brightness = max(xp3, 0);
    for (size_t h = 0; h < 3; ++h) {
        xp3[h] /= (SCALE * brightness + OFFSET);
    }
    color = resized_multichannel(xp3, ArrayShape{color_height, color_width}, target_shape_mode);
    if (source.shape(0) == 3) {
        brightness_and_alpha = resized_singlechannel(
            brightness,
            ArrayShape{structure_height, structure_width},
            target_shape_mode);
    } else if (source.shape(0) == 4) {
        brightness_and_alpha = resized_multichannel(
            Array<float>{brightness, source[3]},
            ArrayShape{structure_height, structure_width},
            target_shape_mode);
    } else {
        throw std::runtime_error("Unsupported number of channels");
    }
}

BrightnessImage::~BrightnessImage() = default;

Array<float> BrightnessImage::reconstructed(
    size_t width,
    size_t height,
    TargetShapeMode target_shape_mode) const
{
    if (color.ndim() != 3) {
        throw std::runtime_error("Unexpected color dimensions");
    }
    if (color.shape(0) != 3) {
        throw std::runtime_error("Unexpected color channels");
    }
    if (brightness_and_alpha.ndim() == 2) {
        width = get_target_shape(brightness_and_alpha.shape(1), width, target_shape_mode);
        height = get_target_shape(brightness_and_alpha.shape(0), height, target_shape_mode);
        auto result = resized_multichannel(color, ArrayShape{height, width}, TargetShapeMode::DEST);
        auto brightness = resized_singlechannel(brightness_and_alpha, ArrayShape{height, width}, TargetShapeMode::DEST);
        for (size_t h = 0; h < color.shape(0); ++h) {
            result[h] *= (SCALE * brightness + OFFSET);
        }
        return clipped(result, 0.f, 1.f);
    } else if (brightness_and_alpha.ndim() == 3) {
        width = get_target_shape(brightness_and_alpha.shape(2), width, target_shape_mode);
        height = get_target_shape(brightness_and_alpha.shape(1), height, target_shape_mode);
        if (brightness_and_alpha.shape(0) != 2) {
            throw std::runtime_error("Unexpected brightness/alpha channels");
        }
        auto result = Array<float>{ArrayShape{4, height, width}};
        result.row_range(0, 3) = resized_multichannel(color, ArrayShape{height, width}, TargetShapeMode::DEST);
        auto ba = resized_multichannel(brightness_and_alpha, ArrayShape{height, width}, TargetShapeMode::DEST);
        for (size_t h = 0; h < 3; ++h) {
            result[h] *= (SCALE * ba[0] + OFFSET);
        }
        result[3] = ba[1];
        return clipped(result, 0.f, 1.f);
    } else {
        throw std::runtime_error("Unexpected brightness/alpha dimensions");
    }
}
