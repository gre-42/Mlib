#include "Crop_Image_File.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

static void crop_image(
    const StbInfo<uint8_t>& source,
    const std::filesystem::path& destination,
    const FixedArray<size_t, 2>& begin,
    const FixedArray<size_t, 2>& end,
    int jpg_quality)
{
    if (any(begin > end)) {
        throw std::runtime_error("begin > end");
    }
    if (end(0) > integral_cast<size_t>(source.height)) {
        throw std::runtime_error("End height too large");
    }
    if (end(1) > integral_cast<size_t>(source.width)) {
        throw std::runtime_error("End width too large");
    }
    auto a = stb_image_2_array(source);
    auto b = a.cropped(
        ArrayShape{0, begin(0), begin(1)},
        ArrayShape{a.shape(0), end(0), end(1)});
    auto dest = StbInfo<uint8_t>{
        integral_cast<int>(end(1) - begin(1)),
        integral_cast<int>(end(0) - begin(0)),
        source.nrChannels};
    array_2_stb_image(b, dest.data());
    if (destination.extension() == ".png") {
        if (!stbi_write_png(destination.c_str(), dest.width, dest.height, dest.nrChannels, dest.data(), 0)) {
            throw std::runtime_error("Could not save to file \"" + destination.string() + '"');
        }
    } else if (destination.extension() == ".jpg") {
        if (!stbi_write_jpg(destination.c_str(), dest.width, dest.height, dest.nrChannels, dest.data(), jpg_quality)) {
            throw std::runtime_error("Could not save to file \"" + destination.string() + '"');
        }
    } else {
        throw std::runtime_error("Filename does not have png or jpg extension: \"" + destination.string() + '"');
    }
}

void Mlib::crop_image_file(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    const FixedArray<size_t, 2>& begin,
    const FixedArray<size_t, 2>& end,
    int jpg_quality)
{
    auto f = stb_load8(source, FlipMode::NONE);
    crop_image(f, destination, begin, end, jpg_quality);
}

void Mlib::crop_image_file(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    const FixedArray<size_t, 2>& size,
    int jpg_quality)
{
    auto f = stb_load8(source, FlipMode::NONE);
    auto center = FixedArray<size_t, 2>{
        integral_cast<size_t>(f.height / 2),
        integral_cast<size_t>(f.width / 2)};
    auto left = size / 2u;
    auto begin = center - left;
    auto end = center + (size - left);
    crop_image(f, destination, begin, end, jpg_quality);
}
