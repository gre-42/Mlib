#include "StbImage3.hpp"
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

StbImage3::StbImage3() = default;

StbImage3::~StbImage3() = default;

StbImage3::StbImage3(const FixedArray<size_t, 2>& shape, const Rgb24& color)
    : Array<Rgb24>(ArrayShape{ shape(0), shape(1) })
{
    Array<Rgb24>& t = *this;
    t = color;
}

StbImage3::StbImage3(const Array<Rgb24>& other)
    : Array<Rgb24>(other)
{}

StbImage3::StbImage3(const FixedArray<size_t, 2>& shape)
    : Array<Rgb24>(ArrayShape{ shape(0), shape(1) }) {}

StbImage3::StbImage3(const StbInfo<uint8_t>& stb_info) {
    if (stb_info.nrChannels != 3) {
        THROW_OR_ABORT("Image does not have 3 channels");
    }
    resize((size_t)stb_info.height, (size_t)stb_info.width);
    memcpy(flat_begin(), stb_info.data(), nbytes());
}

StbImage3 StbImage3::T() const {
    const Array<Rgb24>& t = *this;
    return StbImage3(t.T());
}

StbImage3 StbImage3::reversed(size_t axis) const {
    const Array<Rgb24>& t = *this;
    return StbImage3(t.reversed());
}

void StbImage3::draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void StbImage3::draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color) {
    Mlib::draw_empty_rect(*this, center, size, color);
}

void StbImage3::draw_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Rgb24& color,
    const Rgb24* short_line_color)
{
    draw_line_ext(from, to, thickness, color, false, short_line_color); // false = infinite
}

void StbImage3::draw_infinite_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Rgb24& color,
    const Rgb24* short_line_color)
{
    draw_line_ext(from, to, thickness, color, true, short_line_color); // true = infinite
}

void StbImage3::draw_line_ext(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
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

void StbImage3::draw_mask(const Array<bool>& mask, const Rgb24& color) {
    assert(all(mask.shape() == shape()));
    Array<Rgb24> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void StbImage3::draw_streamline(
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

StbImage3 StbImage3::load_from_file(const std::string& filename) {
    auto image = stb_load8(filename, FlipMode::NONE);
    if (image.nrChannels != 3) {
        THROW_OR_ABORT("Image does not have 3 channels: \"" + filename + '"');
    }
    return StbImage3{image};
}

void StbImage3::save_to_file(const std::string& filename, int jpg_quality) const {
    if (filename.ends_with(".png")) {
        if (!stbi_write_png(filename.c_str(), (int)shape(1), (int)shape(0), 3, flat_begin(), 0)) {
            THROW_OR_ABORT("Could not save to file " + filename);
        }
    } else if (filename.ends_with(".jpg")) {
        if (!stbi_write_jpg(filename.c_str(), (int)shape(1), (int)shape(0), 3, flat_begin(), jpg_quality)) {
            THROW_OR_ABORT("Could not save to file " + filename);
        }
    } else {
        THROW_OR_ABORT("Filename does not have png or jpg extension: \"" + filename + '"');
    }
}

StbImage3 StbImage3::from_rgb(const Array<uint8_t>& rgb) {
    if (rgb.ndim() != 3) {
        THROW_OR_ABORT("from_rgb: rgb image does not have ndim=3, but " + rgb.shape().str());
    }
    if (rgb.shape(0) != 3) {
        THROW_OR_ABORT("from_rgb: rgb image does not have shape(0)=3, but " + rgb.shape().str());
    }
    StbImage3 result(FixedArray<size_t, 2>{ rgb.shape(1), rgb.shape(2) });
    Array<Rgb24> f = result.flattened();
    Array<uint8_t> r = rgb[0].flattened();
    Array<uint8_t> g = rgb[1].flattened();
    Array<uint8_t> b = rgb[2].flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgb24{r(i), g(i), b(i)};
    }
    return result;
}

StbImage3 StbImage3::from_float_rgb(const Array<float>& rgb) {
    if (rgb.ndim() != 3) {
        THROW_OR_ABORT("from_float: rgb image does not have ndim=3, but " + rgb.shape().str());
    }
    if (rgb.shape(0) != 3) {
        THROW_OR_ABORT("from_float: rgb image does not have shape(0)=3, but " + rgb.shape().str());
    }
    StbImage3 result(FixedArray<size_t, 2>{ rgb.shape(1), rgb.shape(2) });
    Array<Rgb24> f = result.flattened();
    Array<float> r = rgb[0].flattened();
    Array<float> g = rgb[1].flattened();
    Array<float> b = rgb[2].flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgb24::from_float_rgb(r(i), g(i), b(i));
    }
    return result;
}

StbImage3 StbImage3::from_float_grayscale(const Array<float>& grayscale) {
    if (grayscale.ndim() != 2) {
        THROW_OR_ABORT("from_float_grayscale: grayscale image does not have ndim=2, but " + grayscale.shape().str());
    }
    StbImage3 result(FixedArray<size_t, 2>{ grayscale.shape(0), grayscale.shape(1) });
    Array<Rgb24> f = result.flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgb24::from_float_grayscale(g(i));
    }
    return result;
}

Array<float> StbImage3::to_float_grayscale() const {
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

Array<float> StbImage3::to_float_rgb() const {
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
