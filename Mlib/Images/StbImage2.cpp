
#include "StbImage2.hpp"
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <fstream>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>
#include <stdexcept>

using namespace Mlib;

StbImage2::StbImage2() = default;

StbImage2::~StbImage2() = default;

StbImage2::StbImage2(const FixedArray<size_t, 2>& shape, const Ia16& color)
    : Array<Ia16>(ArrayShape{ shape(0), shape(1) })
{
    Array<Ia16>& t = *this;
    t = color;
}

StbImage2::StbImage2(const Array<Ia16>& other)
    : Array<Ia16>(other)
{}

StbImage2::StbImage2(const FixedArray<size_t, 2>& shape)
    : Array<Ia16>(ArrayShape{ shape(0), shape(1) }) {}

StbImage2::StbImage2(const StbInfo<uint8_t>& stb_info) {
    if (stb_info.nrChannels != 2) {
        throw std::runtime_error("Image does not have 2 channels");
    }
    resize((size_t)stb_info.height, (size_t)stb_info.width);
    memcpy(flat_begin(), stb_info.data(), nbytes());
}

StbImage2 StbImage2::T() const {
    const Array<Ia16>& t = *this;
    return StbImage2(t.T());
}

StbImage2 StbImage2::reversed(size_t axis) const {
    const Array<Ia16>& t = *this;
    return StbImage2(t.reversed());
}

void StbImage2::draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Ia16& color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void StbImage2::draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Ia16& color) {
    Mlib::draw_empty_rect(*this, center, size, color);
}

void StbImage2::draw_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Ia16& color,
    const Ia16* short_line_color)
{
    draw_line_ext(from, to, thickness, color, false, short_line_color); // false = infinite
}

void StbImage2::draw_infinite_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Ia16& color,
    const Ia16* short_line_color)
{
    draw_line_ext(from, to, thickness, color, true, short_line_color); // true = infinite
}

void StbImage2::draw_line_ext(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Ia16& color,
    bool infinite,
    const Ia16* short_line_color)
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

void StbImage2::draw_mask(const Array<bool>& mask, const Ia16& color) {
    assert(all(mask.shape() == shape()));
    Array<Ia16> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void StbImage2::draw_streamline(
    const FixedArray<size_t, 2>& center,
    const Array<float>& velocity,
    size_t size,
    size_t length,
    const Ia16& color)
{
    visit_streamline(FixedArray<size_t, 2>{ shape(0), shape(1) }, center, velocity, length, [&](const FixedArray<size_t, 2>& ipos){
        draw_fill_rect(ipos, size, color);
    });
}

StbImage2 StbImage2::load_from_file(const std::string& filename) {
    auto image = stb_load8(filename, FlipMode::NONE);
    if (image.nrChannels != 2) {
        throw std::runtime_error("Image does not have 2 channels: \"" + filename + '"');
    }
    return StbImage2{image};
}

void StbImage2::save_to_file(const std::string& filename, int jpg_quality) const {
    if (filename.ends_with(".png")) {
        if (!stbi_write_png(filename.c_str(), (int)shape(1), (int)shape(0), 2, flat_begin(), 0)) {
            throw std::runtime_error("Could not save to file: \"" + filename + '"');
        }
    } else if (filename.ends_with(".jpg")) {
        if (!stbi_write_jpg(filename.c_str(), (int)shape(1), (int)shape(0), 2, flat_begin(), jpg_quality)) {
            throw std::runtime_error("Could not save to file: \"" + filename + '"');
        }
    } else {
        throw std::runtime_error("Filename does not have png or jpg extension: \"" + filename + '"');
    }
}

StbImage2 StbImage2::from_ia(const Array<uint8_t>& ia) {
    if (ia.ndim() != 3) {
        throw std::runtime_error("from_ia: ia image does not have ndim=3, but " + ia.shape().str());
    }
    if (ia.shape(0) != 2) {
        throw std::runtime_error("from_ia: ia image does not have shape(0)=2, but " + ia.shape().str());
    }
    StbImage2 result(FixedArray<size_t, 2>{ ia.shape(1), ia.shape(2) });
    Array<Ia16> f = result.flattened();
    Array<uint8_t> i = ia[0].flattened();
    Array<uint8_t> a = ia[1].flattened();
    for (size_t j = 0; j < i.length(); j++) {
        f(j) = Ia16{i(j), a(j)};
    }
    return result;
}

StbImage2 StbImage2::from_float_ia(const Array<float>& ia) {
    if (ia.ndim() != 3) {
        throw std::runtime_error("from_float_ia: ia image does not have ndim=3, but " + ia.shape().str());
    }
    if (ia.shape(0) != 2) {
        throw std::runtime_error("from_float_ia: ia image does not have shape(0)=2, but " + ia.shape().str());
    }
    StbImage2 result(FixedArray<size_t, 2>{ ia.shape(1), ia.shape(2) });
    Array<Ia16> f = result.flattened();
    Array<float> i = ia[0].flattened();
    Array<float> a = ia[1].flattened();
    for (size_t j = 0; j < i.length(); j++) {
        f(j) = Ia16::from_float_ia(i(j), a(j));
    }
    return result;
}

Array<float> StbImage2::to_float_ia() const {
    Array<float> result(ArrayShape{2}.concatenated(shape()));
    Array<float> I = result[0];
    Array<float> A = result[1];
    for (size_t r = 0; r < shape(0); ++r) {
        for (size_t c = 0; c < shape(1); ++c) {
            I(r, c) = static_cast<float>((*this)(r, c).i) / 255;
            A(r, c) = static_cast<float>((*this)(r, c).a) / 255;
        }
    }
    return result;
}
