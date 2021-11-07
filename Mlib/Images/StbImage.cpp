#include "StbImage.hpp"
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <fstream>
#include <regex>
#include <stb_image/stb_image_load.hpp>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

StbImage::StbImage() {}

StbImage::StbImage(const ArrayShape& shape, const Rgb24& color)
    : Array<Rgb24>(shape)
{
    Array<Rgb24>& t = *this;
    t = color;
}

StbImage::StbImage(const Array<Rgb24>& other)
    : Array<Rgb24>(other)
{}

StbImage::StbImage(const ArrayShape& shape)
    : Array<Rgb24>(shape) {}

StbImage StbImage::T() const {
    const Array<Rgb24>& t = *this;
    return StbImage(t.T());
}

StbImage StbImage::reversed(size_t axis) const {
    const Array<Rgb24>& t = *this;
    return StbImage(t.reversed());
}

void StbImage::draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void StbImage::draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color) {
    Mlib::draw_empty_rect(*this, center, size, color);
}

void StbImage::draw_line(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    const Rgb24& color,
    const Rgb24* short_line_color)
{
    draw_line_ext(from, to, thickness, color, false, short_line_color); // false = infinite
}

void StbImage::draw_infinite_line(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    const Rgb24& color,
    const Rgb24* short_line_color)
{
    draw_line_ext(from, to, thickness, color, true, short_line_color); // true = infinite
}

void StbImage::draw_line_ext(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    const Rgb24& color,
    bool infinite,
    const Rgb24* short_line_color)
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

void StbImage::draw_mask(const Array<bool>& mask, const Rgb24& color) {
    assert(all(mask.shape() == shape()));
    Array<Rgb24> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void StbImage::draw_streamline(
    const FixedArray<size_t, 2>& center,
    const Array<float>& velocity,
    size_t size,
    size_t length,
    const Rgb24& color)
{
    visit_streamline(FixedArray<size_t, 2>{ shape(0), shape(1) }, center, velocity, length, [&](const FixedArray<size_t, 2>& ipos){
        draw_fill_rect(ipos, size, color);
    });
}

StbImage StbImage::load_from_file(const std::string& filename) {
    StbInfo image = stb_load(filename, false, false);
    if (image.nrChannels != 3) {
        throw std::runtime_error("Image does not have 3 channels: \"" + filename + '"');
    }
    StbImage result{ ArrayShape{ (size_t)image.height, (size_t)image.width } };
    memcpy(&result(0, 0), image.data.get(), result.nbytes());
    return result;
}

void StbImage::save_to_file(const std::string& filename, int jpg_quality) const {
    if (filename.ends_with(".png")) {
        if (!stbi_write_png(filename.c_str(), (int)shape(1), (int)shape(0), 3, flat_begin(), 0)) {
            throw std::runtime_error("Could not save to file " + filename);
        }
    } else if (filename.ends_with(".jpg")) {
        if (!stbi_write_jpg(filename.c_str(), (int)shape(1), (int)shape(0), 3, flat_begin(), jpg_quality)) {
            throw std::runtime_error("Could not save to file " + filename);
        }
    } else {
        throw std::runtime_error("Filename does not have png or jpg extension: \"" + filename + '"');
    }
}

StbImage StbImage::from_float_rgb(const Array<float>& rgb) {
    if (rgb.ndim() != 3) {
        throw std::runtime_error("from_float: rgb image does not have ndim=3, but " + rgb.shape().str());
    }
    if (rgb.shape(0) != 3) {
        throw std::runtime_error("from_float: rgb image does not have shape(0)=3, but " + rgb.shape().str());
    }
    StbImage result(rgb.shape().erased_first());
    Array<Rgb24> f = result.flattened();
    Array<float> r = rgb[0].flattened();
    Array<float> g = rgb[1].flattened();
    Array<float> b = rgb[2].flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgb24::from_float_rgb(r(i), g(i), b(i));
    }
    return result;
}

StbImage StbImage::from_float_grayscale(const Array<float>& grayscale) {
    if (grayscale.ndim() != 2) {
        throw std::runtime_error("from_float_grayscale: grayscale image does not have ndim=2, but " + grayscale.shape().str());
    }
    StbImage result(grayscale.shape());
    Array<Rgb24> f = result.flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgb24::from_float_grayscale(g(i));
    }
    return result;
}

Array<float> StbImage::to_float_grayscale() const {
    Array<float> grayscale(shape());
    Array<Rgb24> f = flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        g(i) = (
            static_cast<float>(f(i).r) / 255 +
            static_cast<float>(f(i).g) / 255 +
            static_cast<float>(f(i).b) / 255) / 3;
        assert(g(i) >= 0);
        assert(g(i) <= 1);
    }
    return grayscale;
}

Array<float> StbImage::to_float_rgb() const {
    Array<float> result(ArrayShape{3}.concatenated(shape()));
    Array<float> R = result[0];
    Array<float> G = result[1];
    Array<float> B = result[2];
    for (size_t r = 0; r < shape(0); ++r) {
        for (size_t c = 0; c < shape(1); ++c) {
            R(r, c) = static_cast<float>((*this)(r, c).r) / 255;
            G(r, c) = static_cast<float>((*this)(r, c).g) / 255;
            B(r, c) = static_cast<float>((*this)(r, c).b) / 255;
        }
    }
    return result;
}
