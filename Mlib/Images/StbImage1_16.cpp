
#include "StbImage1_16.hpp"
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <fstream>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>
#include <stb_cpp/stb_image_write_png16.min.hpp>
#include <stdexcept>

using namespace Mlib;

StbImage1_16::StbImage1_16() = default;

StbImage1_16::~StbImage1_16() = default;

StbImage1_16::StbImage1_16(const ArrayShape& shape, uint16_t color)
    : Array<uint16_t>(shape)
{
    Array<uint16_t>& t = *this;
    t = color;
}

StbImage1_16::StbImage1_16(const Array<uint16_t>& other)
    : Array<uint16_t>(other)
{}

StbImage1_16::StbImage1_16(const ArrayShape& shape)
    : Array<uint16_t>(shape) {}

StbImage1_16::StbImage1_16(const StbInfo<uint16_t>& stb_info) {
    if (stb_info.nrChannels != 1) {
        throw std::runtime_error("Image does not have 1 channel");
    }
    resize((size_t)stb_info.height, (size_t)stb_info.width);
    memcpy(flat_begin(), stb_info.data(), nbytes());
}

StbImage1_16 StbImage1_16::T() const {
    const Array<uint16_t>& t = *this;
    return StbImage1_16(t.T());
}

StbImage1_16 StbImage1_16::reversed(size_t axis) const {
    const Array<uint16_t>& t = *this;
    return StbImage1_16(t.reversed());
}

void StbImage1_16::draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, uint16_t color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void StbImage1_16::draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, uint16_t color) {
    Mlib::draw_empty_rect(*this, center, size, color);
}

void StbImage1_16::draw_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    uint16_t color,
    const uint16_t* short_line_color)
{
    draw_line_ext(from, to, thickness, color, false, short_line_color); // false = infinite
}

void StbImage1_16::draw_infinite_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    uint16_t color,
    const uint16_t* short_line_color)
{
    draw_line_ext(from, to, thickness, color, true, short_line_color); // true = infinite
}

void StbImage1_16::draw_line_ext(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    uint16_t color,
    bool infinite,
    const uint16_t* short_line_color)
{
    Mlib::draw_line_ext(
        *this,
        from,
        to,
        thickness,
        color,
        infinite,
        short_line_color);
}

void StbImage1_16::draw_mask(const Array<bool>& mask, uint16_t color) {
    assert(all(mask.shape() == shape()));
    Array<uint16_t> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void StbImage1_16::draw_streamline(
    const FixedArray<size_t, 2>& center,
    const Array<float>& velocity,
    size_t size,
    size_t length,
    uint16_t color)
{
    visit_streamline(FixedArray<size_t, 2>{ shape(0), shape(1) }, center, velocity, length, [&](const FixedArray<size_t, 2>& ipos){
        draw_fill_rect(ipos, size, color);
    });
}

StbImage1_16 StbImage1_16::load_from_file(const Utf8Path& filename) {
    auto vimage = stb_load(filename, FlipMode::NONE);
    if (vimage.index() != 1) {
        throw std::runtime_error("Image does not have 16 bits: \"" + filename.string() + '"');
    }
    const auto& image = std::get<1>(vimage);
    if (image.nrChannels != 1) {
        throw std::runtime_error("Image does not have 1 channel: \"" + filename.string() + '"');
    }
    return StbImage1_16{image};
}

void StbImage1_16::save_to_file(const Utf8Path& filename, int jpg_quality) const {
    if (filename.ends_with(".png")) {
        if (!png16::stbi_write_png16(filename.c_str(), (int)shape(1), (int)shape(0), 1, flat_begin(), 0)) {
            throw std::runtime_error("Could not save to file: \"" + filename.string() + '"');
        }
    } else {
        throw std::runtime_error("Filename does not have png extension: \"" + filename.string() + '"');
    }
}

StbImage1_16 StbImage1_16::from_float_grayscale(const Array<float>& grayscale) {
    if (grayscale.ndim() != 2) {
        throw std::runtime_error("from_float_grayscale: grayscale image does not have ndim=2, but " + grayscale.shape().str());
    }
    StbImage1_16 result(grayscale.shape());
    Array<uint16_t> f = result.flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = uint16_from_float(g(i));
    }
    return result;
}

Array<float> StbImage1_16::to_float_grayscale() const {
    Array<float> grayscale(shape());
    Array<uint16_t> f = flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        g(i) = static_cast<float>(f(i)) / UINT16_MAX;
        assert(g(i) >= 0);
        assert(g(i) <= 1);
    }
    return grayscale;
}
