#pragma once
#include <Mlib/Array/Array.hpp>
#include <cstddef>

namespace Mlib {

enum class TargetShapeMode;

class BrightnessImage {
public:
    BrightnessImage(
        const Array<float>& color,
        const Array<float>& brightness_and_alpha);
    BrightnessImage(
        const Array<float>& source,
        size_t color_width,
        size_t color_height,
        size_t structure_width,
        size_t structure_height,
        TargetShapeMode target_shape_mode);
    ~BrightnessImage();
    Array<float> reconstructed(
        size_t width,
        size_t height,
        TargetShapeMode target_shape_mode) const;
    Array<float> color;
    Array<float> brightness_and_alpha;
};

}
