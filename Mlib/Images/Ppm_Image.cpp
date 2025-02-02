#include "Ppm_Image.hpp"
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using namespace Mlib;

PpmImage::PpmImage() = default;

PpmImage::~PpmImage() = default;

PpmImage::PpmImage(const ArrayShape& shape, const Rgb24& color)
    : Array<Rgb24>(shape)
{
    Array<Rgb24>& t = *this;
    t = color;
}

PpmImage::PpmImage(const Array<Rgb24>& other)
    : Array<Rgb24>(other)
{}

PpmImage::PpmImage(const ArrayShape& shape)
    : Array<Rgb24>(shape) {}

PpmImage PpmImage::T() const {
    const Array<Rgb24>& t = *this;
    return PpmImage(t.T());
}

PpmImage PpmImage::reversed(size_t axis) const {
    const Array<Rgb24>& t = *this;
    return PpmImage(t.reversed());
}

void PpmImage::draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void PpmImage::draw_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Rgb24& color,
    const Rgb24* short_line_color)
{
    draw_line_ext(from, to, thickness, color, false, short_line_color); // false = infinite
}

void PpmImage::draw_infinite_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Rgb24& color,
    const Rgb24* short_line_color)
{
    draw_line_ext(from, to, thickness, color, true, short_line_color); // true = infinite
}

void PpmImage::draw_line_ext(
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

void PpmImage::draw_mask(const Array<bool>& mask, const Rgb24& color) {
    assert(all(mask.shape() == shape()));
    Array<Rgb24> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void PpmImage::draw_streamline(
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

PpmImage PpmImage::load_from_file(const std::string& filename) {
    if (!filename.ends_with(".ppm")) {
        THROW_OR_ABORT("Filename does not have ppm extension: \"" + filename + '"');
    }
    auto istream = create_ifstream(filename, std::ios_base::binary);
    try {
        return load_from_stream(*istream);
    } catch (const std::runtime_error& e) {
        THROW_OR_ABORT(e.what() + std::string(": ") + filename);
    }
}

static void skip_comments(std::istream& istr) {
    if (istr.peek() == '\n') {
        istr.get();
    }
    if (istr.peek() == '#') {
        std::string line;
        std::getline(istr, line);
    }
}

PpmImage PpmImage::load_from_stream(std::istream& istream) {
    PpmImage result;
    std::string header;
    istream >> header;
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read header");
    }
    if (header != "P6") {
        THROW_OR_ABORT("Header does not equal P6");
    }
    skip_comments(istream);
    size_t width;
    istream >> width;
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read width");
    }
    size_t height;
    istream >> height;
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read height");
    }
    size_t n255;
    istream >> n255;
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read maximum value");
    }
    if (n255 != 255) {
        THROW_OR_ABORT("Maximum value does not equal 255");
    }
    char c;
    istream.read(&c, 1);
    if (istream.fail() || (c != '\n')) {
        THROW_OR_ABORT("Could not read newline");
    }
    result.do_resize(ArrayShape{height, width});
    istream.read(reinterpret_cast<char*>(&result(0, 0)), integral_cast<std::streamsize>(result.nbytes()));
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read raw image");
    }
    return result;
}

void PpmImage::save_to_file(const std::string& filename) const {
    if (!filename.ends_with(".ppm")) {
        THROW_OR_ABORT("Filename does not have ppm extension: \"" + filename + '"');
    }
    try {
        std::ofstream ostream(filename, std::ios::binary);
        save_to_stream(ostream);
    } catch (const std::runtime_error& e) {
        THROW_OR_ABORT("Could not save to file " + filename + "; " + e.what());
    }
}

void PpmImage::save_to_stream(std::ostream& ostream) const {
    if (ndim() != 2) {
        THROW_OR_ABORT("save_to_stream: image does not have ndim=2, but " + shape().str());
    }
    std::string header{"P6\n" + std::to_string(shape(1)) + " " + std::to_string(shape(0)) + "\n255\n"};
    ostream.write(header.c_str(), integral_cast<std::streamsize>(header.length()));
    ostream.write((const char*)flat_iterable().begin(), integral_cast<std::streamsize>(nbytes()));
    ostream.flush();
    if (ostream.fail()) {
        THROW_OR_ABORT("Could not write PPM");
    }
}

PpmImage PpmImage::from_float_rgb(const Array<float>& rgb) {
    if (rgb.ndim() != 3) {
        THROW_OR_ABORT("from_float: rgb image does not have ndim=3, but " + rgb.shape().str());
    }
    if (rgb.shape(0) != 3) {
        THROW_OR_ABORT("from_float: rgb image does not have shape(0)=3, but " + rgb.shape().str());
    }
    PpmImage result(rgb.shape().erased_first());
    Array<Rgb24> f = result.flattened();
    Array<float> r = rgb[0].flattened();
    Array<float> g = rgb[1].flattened();
    Array<float> b = rgb[2].flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgb24::from_float_rgb(r(i), g(i), b(i));
    }
    return result;
}

PpmImage PpmImage::from_float_grayscale(const Array<float>& grayscale) {
    if (grayscale.ndim() != 2) {
        THROW_OR_ABORT("from_float_grayscale: grayscale image does not have ndim=2, but " + grayscale.shape().str());
    }
    PpmImage result(grayscale.shape());
    Array<Rgb24> f = result.flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Rgb24::from_float_grayscale(g(i));
    }
    return result;
}

Array<float> PpmImage::to_float_grayscale() const {
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

Array<float> PpmImage::to_float_rgb() const {
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
