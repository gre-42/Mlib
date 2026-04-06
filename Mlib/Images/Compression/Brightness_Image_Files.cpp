#include "Brightness_Image_Files.hpp"
#include <Mlib/Images/Get_Target_Shape.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage2.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

static Array<float> load_reconstructed_as_array(const std::string& filename, FlipMode flip_mode) {
    auto image = stb_load8(filename, flip_mode);
    if (image.nrChannels == 3) {
        return StbImage3{image}.to_float_rgb();
    } else if (image.nrChannels == 4) {
        return StbImage4{image}.to_float_rgba();
    } else {
        throw std::runtime_error("Unsupported number of channels: \"" + filename + '"');
    }
}

static Array<float> load_luma_as_array(const std::string& filename, FlipMode flip_mode) {
    auto image = stb_load8(filename, flip_mode);
    if (image.nrChannels == 1) {
        return StbImage1{image}.to_float_grayscale();
    } else if (image.nrChannels == 2) {
        return StbImage2{image}.to_float_ia();
    } else {
        throw std::runtime_error("Unsupported number of channels: \"" + filename + '"');
    }
}

static Array<float> load_chrominance_as_array(const std::string& filename, FlipMode flip_mode) {
    auto image = stb_load8(filename, flip_mode);
    if (image.nrChannels == 3) {
        return StbImage3{image}.to_float_rgb();
    } else {
        throw std::runtime_error("Unsupported number of channels: \"" + filename + '"');
    }
}

BrightnessImageFiles::BrightnessImageFiles(
    const std::string& source,
    size_t color_width,
    size_t color_height,
    size_t structure_width,
    size_t structure_height,
    TargetShapeMode target_shape_mode)
    : brightness_image{
        load_reconstructed_as_array(source, FlipMode::NONE),
        color_width,
        color_height,
        structure_width,
        structure_height,
        target_shape_mode}
{}

void BrightnessImageFiles::save_reconstructed(const std::string& filename) const {
    const auto& ba = brightness_image.brightness_and_alpha;
    if (ba.ndim() == 2) {
        StbImage3::from_float_rgb(clipped(brightness_image.reconstructed(ba.shape(1), ba.shape(0), TargetShapeMode::DEST), 0.f, 1.f)).save_to_file(filename);
    } else if (ba.ndim() == 3) {
        StbImage4::from_float_rgba(clipped(brightness_image.reconstructed(ba.shape(2), ba.shape(1), TargetShapeMode::DEST), 0.f, 1.f)).save_to_file(filename);
    } else {
        throw std::runtime_error("Unsupported number of channels");
    }
}

void BrightnessImageFiles::save_color(const std::string& filename) const {
    StbImage3::from_float_rgb(clipped(brightness_image.color, 0.f, 1.f)).save_to_file(filename);
}

void BrightnessImageFiles::save_brightness_and_alpha(const std::string& filename) const {
    const auto& ba = brightness_image.brightness_and_alpha;
    if (ba.ndim() == 2) {
        StbImage1::from_float_grayscale(clipped(ba, 0.f, 1.f)).save_to_file(filename);
    } else if (ba.ndim() == 3) {
        StbImage2::from_float_ia(clipped(ba, 0.f, 1.f)).save_to_file(filename);
    } else {
        throw std::runtime_error("Unsupported number of channels");
    }
}

StbInfo<uint8_t> Mlib::reconstruct_from_luma_and_chrominance(
    const std::string& luma,
    const std::string& chrominance,
    FlipMode flip_mode)
{
    BrightnessImage bi{
        load_chrominance_as_array(chrominance, flip_mode),
        load_luma_as_array(luma, flip_mode)};
    auto recon = [&](){
        switch (bi.brightness_and_alpha.ndim()) {
        case 2:
            return bi.reconstructed(bi.brightness_and_alpha.shape(1), bi.brightness_and_alpha.shape(0), TargetShapeMode::DEST);
        case 3:
            return bi.reconstructed(bi.brightness_and_alpha.shape(2), bi.brightness_and_alpha.shape(1), TargetShapeMode::DEST);
        }
        throw std::runtime_error("Unexpected array dimensionality");
    }();
    auto result = StbInfo<uint8_t>{
        integral_cast<int>(recon.shape(2)),
        integral_cast<int>(recon.shape(1)),
        integral_cast<int>(recon.shape(0))};
    array_2_stb_image(round(recon * 255.f).casted<uint8_t>(), result.data());
    return result;
}
