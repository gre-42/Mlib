#include "StbImage4.hpp"
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <fstream>
#include <regex>
#include <stb_image/stb_image_load.hpp>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

StbImage4::StbImage4() {}

StbImage4::StbImage4(const ArrayShape& shape, const Rgba32& color)
    : Array<Rgba32>(shape)
{
    Array<Rgba32>& t = *this;
    t = color;
}

StbImage4::StbImage4(const Array<Rgba32>& other)
    : Array<Rgba32>(other)
{}

StbImage4::StbImage4(const ArrayShape& shape)
    : Array<Rgba32>(shape) {}

StbImage4 StbImage4::T() const {
    const Array<Rgba32>& t = *this;
    return StbImage4(t.T());
}

StbImage4 StbImage4::reversed(size_t axis) const {
    const Array<Rgba32>& t = *this;
    return StbImage4(t.reversed());
}

void StbImage4::draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgba32& color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void StbImage4::draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgba32& color) {
    Mlib::draw_empty_rect(*this, center, size, color);
}

void StbImage4::draw_line(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    const Rgba32& color,
    const Rgba32* short_line_color)
{
    draw_line_ext(from, to, thickness, color, false, short_line_color); // false = infinite
}

void StbImage4::draw_infinite_line(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    const Rgba32& color,
    const Rgba32* short_line_color)
{
    draw_line_ext(from, to, thickness, color, true, short_line_color); // true = infinite
}

void StbImage4::draw_line_ext(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    const Rgba32& color,
    bool infinite,
    const Rgba32* short_line_color)
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

void StbImage4::draw_mask(const Array<bool>& mask, const Rgba32& color) {
    assert(all(mask.shape() == shape()));
    Array<Rgba32> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void StbImage4::draw_streamline(
    const FixedArray<size_t, 2>& center,
    const Array<float>& velocity,
    size_t size,
    size_t length,
    const Rgba32& color)
{
    visit_streamline(FixedArray<size_t, 2>{ shape(0), shape(1) }, center, velocity, length, [&](const FixedArray<size_t, 2>& ipos){
        draw_fill_rect(ipos, size, color);
    });
}

StbImage4 StbImage4::load_from_file(const std::string& filename) {
    StbInfo image = stb_load(filename, false, false);
    if (image.nrChannels != 4) {
        throw std::runtime_error("Image does not have 4 channels: \"" + filename + '"');
    }
    StbImage4 result{ ArrayShape{ (size_t)image.height, (size_t)image.width } };
    memcpy(&result(0, 0), image.data.get(), result.nbytes());
    return result;
}

void StbImage4::save_to_file(const std::string& filename, int jpg_quality) const {
    if (filename.ends_with(".png")) {
        if (!stbi_write_png(filename.c_str(), (int)shape(1), (int)shape(0), 4, flat_begin(), 0)) {
            throw std::runtime_error("Could not save to file " + filename);
        }
    } else {
        throw std::runtime_error("Filename does not have png extension: \"" + filename + '"');
    }
}

StbImage4 StbImage4::from_float_rgba(const Array<float>& rgba) {
    if (rgba.ndim() != 3) {
        throw std::runtime_error("from_float: rgba image does not have ndim=4, but " + rgba.shape().str());
    }
    if (rgba.shape(0) != 4) {
        throw std::runtime_error("from_float: rgba image does not have shape(0)=4, but " + rgba.shape().str());
    }
    StbImage4 result(rgba.shape().erased_first());
    Array<Rgba32> f = result.flattened();
    Array<float> r = rgba[0].flattened();
    Array<float> g = rgba[1].flattened();
    Array<float> b = rgba[2].flattened();
    Array<float> a = rgba[3].flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgba32::from_float_rgba(r(i), g(i), b(i), a(i));
    }
    return result;
}

Array<float> StbImage4::to_float_rgba() const {
    Array<float> result(ArrayShape{4}.concatenated(shape()));
    Array<float> R = result[0];
    Array<float> G = result[1];
    Array<float> B = result[2];
    Array<float> A = result[3];
    for (size_t r = 0; r < shape(0); ++r) {
        for (size_t c = 0; c < shape(1); ++c) {
            R(r, c) = static_cast<float>((*this)(r, c).r) / 255;
            G(r, c) = static_cast<float>((*this)(r, c).g) / 255;
            B(r, c) = static_cast<float>((*this)(r, c).b) / 255;
            A(r, c) = static_cast<float>((*this)(r, c).a) / 255;
        }
    }
    return result;
}
