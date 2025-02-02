#include "Bgr565Bitmap.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using namespace Mlib;

/*static void print_bytes(std::istream& istream, size_t nbytes) {
    size_t ct = 0;
    while (ct < nbytes) {
        if (istream.fail() || istream.eof()) {
            THROW_OR_ABORT("Could not read from stream");
        }
        unsigned char hp = istream.get();
        std::cout << ct << ": " << (unsigned int)hp << std::endl;
        ct++;
    }
}

template <class T>
static void print_bytes(const T& header) {
    for (size_t ct = 0; ct < sizeof(header); ct++)
    {
        unsigned char value = reinterpret_cast<const unsigned char*>(&header)[ct];
        std::cout << ct << ": " << (unsigned int)value << std::endl;
    }
}*/

static Bgr565Bitmap aligned_bitmap(const ArrayShape& shape) {
    return Bgr565Bitmap{ArrayShape{shape(0), ((shape(1) + 1u) & size_t(-2))}};
}

Bgr565Bitmap::Bgr565Bitmap() {}

Bgr565Bitmap::Bgr565Bitmap(const ArrayShape& shape, const Bgr565& color)
    : Array<Bgr565>(shape)
{
    Array<Bgr565>& t = *this;
    t = color;
}

Bgr565Bitmap::Bgr565Bitmap(const Array<Bgr565>& other)
    : Array<Bgr565>(other)
{}

Bgr565Bitmap::Bgr565Bitmap(const ArrayShape& shape)
    : Array<Bgr565>(shape) {}

void Bgr565Bitmap::draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Bgr565& color) {
    Mlib::draw_fill_rect(*this, center, size, color);
}

void Bgr565Bitmap::draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Bgr565& color) {
    Mlib::draw_empty_rect(*this, center, size, color);
}

void Bgr565Bitmap::draw_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Bgr565& color)
{
    draw_line_ext(from, to, thickness, color, false); // false = infinite
}

void Bgr565Bitmap::draw_infinite_line(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Bgr565& color)
{
    draw_line_ext(from, to, thickness, color, true); // true = infinite
}

void Bgr565Bitmap::draw_line_ext(
    const FixedArray<float, 2>& from,
    const FixedArray<float, 2>& to,
    size_t thickness,
    const Bgr565& color,
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

void Bgr565Bitmap::draw_mask(const Array<bool>& mask, const Bgr565& color) {
    assert(all(mask.shape() == shape()));
    Array<Bgr565> f = flattened();
    Array<bool> m = mask.flattened();
    for (size_t i = 0; i < f.length(); ++i) {
        if (!m(i)) {
            f(i) = color;
        }
    }
}

void Bgr565Bitmap::draw_streamline(
    const FixedArray<size_t, 2>& center,
    const Array<float>& velocity,
    size_t size,
    size_t length,
    const Bgr565& color)
{
    visit_streamline(FixedArray<size_t, 2>{ shape(0), shape(1) }, center, velocity, length, [&](const FixedArray<size_t, 2>& ipos){
        draw_fill_rect(ipos, size, color);
    });
}

Bgr565Bitmap Bgr565Bitmap::load_from_file(const std::string& filename) {
    try {
        auto istream = create_ifstream(filename, std::ios_base::binary);
        return load_from_stream(*istream);
    } catch (const std::runtime_error& e) {
        THROW_OR_ABORT("Could not load from file " + filename + "; " + e.what());
    }
}

void Bgr565Bitmap::save_to_file(const std::string& filename) const {
    try {
        std::ofstream ostream(filename, std::ios_base::binary);
        save_to_stream(ostream);
    } catch (const std::runtime_error& e) {
        THROW_OR_ABORT("Could not save to file " + filename + "; " + e.what());
    }
}

Bgr565Bitmap Bgr565Bitmap::load_from_stream(std::istream& istream) {
    BitmapHeader header;
    assert_true((unsigned char*)&header.width - (unsigned char*)&header == 18);
    assert_true((unsigned char*)&header.height - (unsigned char*)&header == 22);
    //print_bytes(istream, 30);
    assert_true(sizeof(header) == 54);
    istream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read bitmap header");
    }
    if (header.type != 0x4d42) {
        THROW_OR_ABORT("Bitmap file type not supported (must be rgb=5x6x5)");
    }
    // print_bytes(header);
    if (header.offBytes < sizeof(header)) {
        THROW_OR_ABORT("Bitmap offbytes smaller than header");
    }
    std::vector<unsigned char> off_header;
    off_header.resize(header.offBytes - sizeof(header));
    istream.read(reinterpret_cast<char*>(&off_header[0]), integral_cast<std::streamsize>(off_header.size()));
    if (off_header.size() != sizeof(off_bitmap_header_565)) {
        THROW_OR_ABORT("File format not supported (offset mismatch)");
    }
    for (size_t i = 0; i < sizeof(off_bitmap_header_565); i++) {
        if (off_header[i] != off_bitmap_header_565[i]) {
            THROW_OR_ABORT("File format not supported in index " +
                std::to_string(i));
        }
    }
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read bitmap offBytes");
    }
    static_assert(sizeof(Bgr565) == 2);
    Bgr565Bitmap aligned{aligned_bitmap(ArrayShape{header.height, header.width})};
    if (aligned.nbytes() != header.sizeImage) {
        // lerr() << aligned.nbytes();
        // lerr() << header.sizeImage;
        THROW_OR_ABORT("Image size does not match padding");
    }
    istream.read(reinterpret_cast<char*>(&aligned(0, 0)), integral_cast<std::streamsize>(aligned.nbytes()));
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read bitmap data");
    }
    Bgr565Bitmap result(ArrayShape{header.height, header.width});
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            result(result.shape(0) - 1 - r, c) = aligned(r, c);
        }
    }
    return result;
}

void Bgr565Bitmap::save_to_stream(std::ostream& ostream) const {
    if (ndim() != 2) {
        THROW_OR_ABORT("save_to_stream: image does not have ndim=2, but " + shape().str());
    }
    Bgr565Bitmap aligned{aligned_bitmap(shape())};
    for (size_t r = 0; r < shape(0); ++r) {
        for (size_t c = 0; c < shape(1); ++c) {
            aligned(shape(0) - 1 - r, c) = (*this)(r, c);
        }
        if (aligned.shape(1) != shape(1)) {
            aligned(shape(0) - 1 - r, aligned.shape(1) - 1) = Bgr565::white();
        }
    }
    BitmapHeader header;
    header.type = 0x4d42;
    header.fileSize = sizeof(off_bitmap_header_565) + sizeof(header) + (unsigned int)aligned.nbytes();
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offBytes = sizeof(off_bitmap_header_565) + sizeof(header);
    header.size = 0x7c;
    header.width = (unsigned int)shape(1);
    header.height = (unsigned int)shape(0);
    header.planes = 0x1;
    header.bitCount = 0x10;
    header.compression = 0x3;
    header.sizeImage = (unsigned int)aligned.nbytes();
    header.xPelsPerMeter = 0xb13;
    header.yPelsPerMeter = 0xb13;
    header.clrUsed = 0;
    header.clrImportant = 0;
    assert_true(sizeof(header) == 54);
    assert_true(sizeof(off_bitmap_header_565) == 84);
    ostream.write(
        reinterpret_cast<const char*>(&header),
        sizeof(header));
    if (ostream.fail()) {
        THROW_OR_ABORT("Could not save bitmap header");
    }
    ostream.write(
        reinterpret_cast<const char*>(&off_bitmap_header_565[0]),
        sizeof(off_bitmap_header_565));
    if (ostream.fail()) {
        THROW_OR_ABORT("Could not save bitmap header");
    }
    ostream.write(
        reinterpret_cast<const char*>(&aligned(0, 0)),
        integral_cast<std::streamsize>(aligned.nbytes()));
    ostream.flush();
    if (ostream.fail()) {
        THROW_OR_ABORT("Could not save bitmap data");
    }
}

Bgr565Bitmap Bgr565Bitmap::from_float_rgb(const Array<float>& rgb) {
    if (rgb.ndim() != 3) {
        THROW_OR_ABORT("from_float: rgb image does not have ndim=3, but " + rgb.shape().str());
    }
    if (rgb.shape(0) != 3) {
        THROW_OR_ABORT("from_float: rgb image does not have shape(0)=3, but " + rgb.shape().str());
    }
    Bgr565Bitmap result(rgb.shape().erased_first());
    Array<Bgr565> f = result.flattened();
    Array<float> r = rgb[0].flattened();
    Array<float> g = rgb[1].flattened();
    Array<float> b = rgb[2].flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Bgr565::from_float_rgb(r(i), g(i), b(i));
    }
    return result;
}

Bgr565Bitmap Bgr565Bitmap::from_float_grayscale(const Array<float>& grayscale) {
    if (grayscale.ndim() != 2) {
        THROW_OR_ABORT("from_float_grayscale: grayscale image does not have ndim=2, but " + grayscale.shape().str());
    }
    Bgr565Bitmap result(grayscale.shape());
    Array<Bgr565> f = result.flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        f(i) = Bgr565::from_float_grayscale(g(i));
    }
    return result;
}

Array<float> Bgr565Bitmap::to_float_grayscale() const {
    if (ndim() != 2) {
        THROW_OR_ABORT("to_float_grayscale: image does not have ndim=2, but " + shape().str());
    }
    Array<float> grayscale(shape());
    Array<Bgr565> f = flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        g(i) = (
            static_cast<float>(f(i).r) / ((1 << 5) - 1) +
            static_cast<float>(f(i).g) / ((1 << 6) - 1) +
            static_cast<float>(f(i).b) / ((1 << 5) - 1)) / 3;
        assert(g(i) >= 0);
        assert(g(i) <= 1);
    }
    return grayscale;
}

Array<float> Bgr565Bitmap::to_float_rgb() const {
    Array<float> result(ArrayShape{3}.concatenated(shape()));
    Array<float> R = result[0];
    Array<float> G = result[1];
    Array<float> B = result[2];
    for (size_t r = 0; r < shape(0); ++r) {
        for (size_t c = 0; c < shape(1); ++c) {
            R(r, c) = static_cast<float>((*this)(r, c).r) / ((1 << 5) - 1);
            G(r, c) = static_cast<float>((*this)(r, c).g) / ((1 << 6) - 1);
            B(r, c) = static_cast<float>((*this)(r, c).b) / ((1 << 5) - 1);
        }
    }
    return result;
}
