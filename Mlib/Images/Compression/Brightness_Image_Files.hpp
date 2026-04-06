#pragma once
#include <Mlib/Images/Compression/Brightness_Image.hpp>
#include <Mlib/Images/Flip_Mode.hpp>
#include <cstdint>
#include <string>

template <class TData>
class StbInfo;

namespace Mlib {

class BrightnessImageFiles {
public:
    BrightnessImageFiles(
        const std::string& source,
        size_t color_width,
        size_t color_height,
        size_t structure_width,
        size_t structure_height,
        TargetShapeMode target_shape_mode);
    void save_reconstructed(const std::string& filename) const;
    void save_color(const std::string& filename) const;
    void save_brightness_and_alpha(const std::string& filename) const;
    BrightnessImage brightness_image;
};

StbInfo<uint8_t> reconstruct_from_luma_and_chrominance(
    const std::string& luma,
    const std::string& chrominance,
    FlipMode flip_mode = FlipMode::NONE);

}
