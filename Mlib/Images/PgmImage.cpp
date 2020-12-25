#include "PgmImage.hpp"
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <fstream>
#include <regex>

using namespace Mlib;

PgmImage::PgmImage() {}

PgmImage::PgmImage(const ArrayShape& shape, uint16_t color)
    : Array<uint16_t>(shape)
{
    Array<uint16_t>& t = *this;
    t = color;
}

PgmImage::PgmImage(const Array<uint16_t>& other)
    : Array<uint16_t>(other)
{}

PgmImage::PgmImage(const ArrayShape& shape)
    : Array<uint16_t>(shape) {}

void PgmImage::draw_fill_rect(const ArrayShape& center, size_t size, uint16_t color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void PgmImage::draw_line(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    uint16_t color)
{
    draw_line_ext(from, to, thickness, color, false); // false = infinite
}

void PgmImage::draw_infinite_line(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    uint16_t color)
{
    draw_line_ext(from, to, thickness, color, true); // true = infinite
}

void PgmImage::draw_line_ext(
    const Array<float>& from,
    const Array<float>& to,
    size_t thickness,
    uint16_t color,
    bool infinite)
{
    Mlib::draw_line_ext(
        *this,
        from,
        to,
        thickness,
        color,
        infinite);
}

void PgmImage::draw_mask(const Array<bool>& mask, uint16_t color) {
    assert(all(mask.shape() == shape()));
    Array<uint16_t> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void PgmImage::draw_streamline(
    const ArrayShape& center,
    const Array<float>& velocity,
    size_t size,
    size_t length,
    uint16_t color)
{
    visit_streamline(shape(), center, velocity, length, [&](const ArrayShape& ipos){
        draw_fill_rect(ipos, size, color);
    });
}

PgmImage PgmImage::load_from_file(const std::string& filename) {
    std::ifstream istream(filename, std::ios_base::binary);
    try {
        return load_from_stream(istream);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(e.what() + std::string(": ") + filename);
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

PgmImage PgmImage::load_from_stream(std::istream& istream) {
    PgmImage result;
    std::string header;
    istream >> header;
    if (istream.fail()) {
        throw std::runtime_error("Could not read header");
    }
    if (header != "P5") {
        throw std::runtime_error("Header does not equal P5");
    }
    skip_comments(istream);
    size_t width;
    istream >> width;
    if (istream.fail()) {
        throw std::runtime_error("Could not read width");
    }
    size_t height;
    istream >> height;
    if (istream.fail()) {
        throw std::runtime_error("Could not read height");
    }
    size_t nUINT16_MAX;
    istream >> nUINT16_MAX;
    if (istream.fail()) {
        throw std::runtime_error("Could not read maximum value");
    }
    if (nUINT16_MAX != UINT16_MAX) {
        throw std::runtime_error("Maximum value does not equal 65535");
    }
    char c;
    istream.read(&c, 1);
    if (istream.fail() || (c != '\n')) {
        throw std::runtime_error("Could not read newline");
    }
    result.do_resize(ArrayShape{height, width});
    istream.read(reinterpret_cast<char*>(&result(0, 0)), result.nbytes());
    for (auto& v : result.flat_iterable()) {
        v = ((v & 0xFF00) >> 8) | ((v & 0xFF) << 8);
    }
    if (istream.fail()) {
        throw std::runtime_error("Could not read pgm image");
    }
    return result;
}

void PgmImage::save_to_file(const std::string& filename) const {
    try {
        std::ofstream ostream(filename, std::ios::binary);
        save_to_stream(ostream);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Could not save to file " + filename + "; " + e.what());
    }
}

void PgmImage::save_to_stream(std::ostream& ostream) const {
    if (ndim() != 2) {
        throw std::runtime_error("save_to_stream: image does not have ndim=2, but " + shape().str());
    }
    std::string header{"P5\n" + std::to_string(shape(1)) + " " + std::to_string(shape(0)) + "\n65535\n"};
    ostream.write(header.c_str(), header.length());
    for (auto v : flat_iterable()) {
        ostream.put((v & 0xFF00) >> 8);
        ostream.put(v & 0xFF);
    }
    ostream.flush();
    if (ostream.fail()) {
        throw std::runtime_error("Could not write PGM");
    }
}

PgmImage PgmImage::from_float(const Array<float>& grayscale) {
    if (grayscale.ndim() != 2) {
        throw std::runtime_error("from_float: grayscale image does not have ndim=2, but " + grayscale.shape().str());
    }
    PgmImage result(grayscale.shape());
    Array<uint16_t> f = result.flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = uint16_from_float(g(i));
    }
    return result;
}

Array<float> PgmImage::to_float() const {
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
