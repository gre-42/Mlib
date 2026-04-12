#include "Resize_File.hpp"
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Pgm_Image.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage1_16.hpp>
#include <Mlib/Images/StbImage2.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Images/Transform/Resize.hpp>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>

void Mlib::resize_file(
    const Utf8Path& source,
    const Utf8Path& dest,
    const FixedArray<size_t, 2>& target_size,
    FilterExtension filter_extension,
    TargetShapeMode target_shape_mode,
    int jpg_quality)
{
    auto atarget_shape = ArrayShape{target_size(1), target_size(0)};
    auto save_8 = [&](const Array<uint8_t>& array){
        if (array.ndim() != 3) {
            throw std::runtime_error("Unexpected array dimensionality");
        }
        auto resized = [&](){
            if (array.shape(0) == 1) {
                return resized_singlechannel(array[0].casted<float>() / 255.f, atarget_shape, target_shape_mode, filter_extension);
            } else {
                return resized_multichannel(array.casted<float>() / 255.f, atarget_shape, target_shape_mode, filter_extension);
            }
        }();
        clip(resized, 0.f, 1.f);
        switch (array.shape(0)) {
        case 1:
            StbImage1::from_float_grayscale(resized).save_to_file(dest, jpg_quality);
            break;
        case 2:
            StbImage2::from_float_ia(resized).save_to_file(dest, jpg_quality);
            break;
        case 3:
            StbImage3::from_float_rgb(resized).save_to_file(dest, jpg_quality);
            break;
        case 4:
            StbImage4::from_float_rgba(resized).save_to_file(dest, jpg_quality);
            break;
        default:
            throw std::runtime_error("Unsupported number of channels: \"" + source.string() + '"');
        }
    };
    auto save_16 = [&](const Array<uint16_t>& array){
        if (array.ndim() != 3) {
            throw std::runtime_error("Unexpected array dimensionality");
        }
        auto resized = [&](){
            if (array.shape(0) == 1) {
                return resized_singlechannel(array[0].casted<float>() / 65535.f, atarget_shape, target_shape_mode, filter_extension);
            } else {
                return resized_multichannel(array.casted<float>() / 65535.f, atarget_shape, target_shape_mode, filter_extension);
            }
        }();
        clip(resized, 0.f, 1.f);
        switch (array.shape(0)) {
        case 1:
            StbImage1_16::from_float_grayscale(resized).save_to_file(dest);
            break;
        default:
            throw std::runtime_error("Unsupported number of channels: \"" + source.string() + '"');
        }
    };
    if (source.extension() == ".pgm") {
        auto g = PgmImage::load_from_file(source);
        g.reshape(ArrayShape{1}.concatenated(g.shape()));
        save_16(g);
    } else {
        auto f = stb_load(source, FlipMode::NONE);
        if (f.index() == 0) {
            const auto& f8 = std::get<0>(f);
            save_8(stb_image_2_array(f8));
        } else {
            const auto& f16 = std::get<1>(f);
            save_16(stb_image_2_array(f16));
        }
    }
}
