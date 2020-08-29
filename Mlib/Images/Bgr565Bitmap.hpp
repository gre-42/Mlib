#pragma once
#include <Mlib/Array/Array.hpp>
#include <string>

namespace Mlib {

struct BitmapHeader {
    // File info
    unsigned short type;        // 0
    unsigned int fileSize;      // 2
    unsigned short reserved1;   // 6
    unsigned short reserved2;   // 8
    unsigned int offBytes;      // 10
    // Bitmap info
    unsigned int size;          // 14
    unsigned int width;         // 18
    unsigned int height;        // 22
    unsigned short planes;      // 26
    unsigned short bitCount;    // 28
    unsigned int compression;   // 30
    unsigned int sizeImage;     // 34
    unsigned int xPelsPerMeter; // 38
    unsigned int yPelsPerMeter; // 42
    unsigned int clrUsed;       // 46
    unsigned int clrImportant;  // 50 - 54
} __attribute__((packed));

static const unsigned char off_bitmap_header_565[] = {
    0, 0xf8, 0, 0, 0xe0, 0x7, 0, 0, 0x1f, 0, 0, 0, 0, 0, 0, 0,
    0x42, 0x47, 0x52, 0x73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0};

// 16-bit, 2 * 8 bytes
struct Bgr565 {
    unsigned short b:5;
    unsigned short g:6;
    unsigned short r:5;
    Bgr565() {}
    Bgr565(
        unsigned short r,
        unsigned short g,
        unsigned short b)
    :b(b), g(g), r(r) {}
    static inline Bgr565 red() {
        return Bgr565{
            (1 << 5) - 1,
            0,
            0};
    }
    static inline Bgr565 green() {
        return Bgr565{
            0,
            (1 << 6) - 1,
            0};
    }
    static inline Bgr565 blue() {
        return Bgr565{
            0,
            0,
            (1 << 5) - 1};
    }
    static inline Bgr565 black() {
        return Bgr565{0, 0, 0};
    }
    static inline Bgr565 white() {
        return Bgr565{
            (1 << 5) - 1,
            (1 << 6) - 1,
            (1 << 5) - 1};
    }
    static inline Bgr565 nan() {
        return Bgr565{
            (1 << 5) - 1,
            0,
            (1 << 5) - 1};
    }
    static inline Bgr565 from_float_grayscale(float grayscale) {
        if (std::isnan(grayscale)) {
            return Bgr565::nan();
        }
        // consider using grayscale.clip(0, 1) if this fails
        if (grayscale < 0) {
            throw std::runtime_error("Bgr565Bitmap::from_float_grayscale received " + std::to_string(grayscale) + "<0");
        }
        if (grayscale > 1) {
            throw std::runtime_error("Bgr565Bitmap::from_float_grayscale received " + std::to_string(grayscale) + ">1");
        }
        return Bgr565{
            (unsigned short)(grayscale * ((1 << 5) - 1) + 0.5f),
            (unsigned short)(grayscale * ((1 << 6) - 1) + 0.5f),
            (unsigned short)(grayscale * ((1 << 5) - 1) + 0.5f)};
    }
    static inline Bgr565 from_float_rgb(const Array<float>& rgb) {
        if (any(isnan(rgb))) {
            if (!all(isnan(rgb))) {
                throw std::runtime_error("Bgr565Bitmap::from_float_rgb received inconsistent NANs");
            }
            return Bgr565::nan();
        }
        assert(all(rgb.shape() == ArrayShape{3}));
        if (any(rgb < 0.f)) {
            throw std::runtime_error("Bgr565Bitmap::from_float_rgb received " + rgb.str() + "<0");
        }
        if (any(rgb > 1.f)) {
            throw std::runtime_error("Bgr565Bitmap::from_float_rgb received " + rgb.str() + ">1");
        }
        return Bgr565{
            (unsigned short)(rgb(0) * ((1 << 5) - 1) + 0.5f),
            (unsigned short)(rgb(1) * ((1 << 6) - 1) + 0.5f),
            (unsigned short)(rgb(2) * ((1 << 5) - 1) + 0.5f)};
    }
    static inline Bgr565 from_float_rgb(float r, float g, float b) {
        if (std::isnan(r) || std::isnan(g) || std::isnan(b)) {
            if (!std::isnan(r) || !std::isnan(g) || !std::isnan(b)) {
                throw std::runtime_error("Bgr565Bitmap::from_float_rgb received inconsistent NANs");
            }
            return Bgr565::nan();
        }
        if (r < 0.f || g < 0.f || b < 0.f) {
            throw std::runtime_error("Bgr565Bitmap::from_float_rgb received value < 0");
        }
        if (r > 1.f || g > 1.f || b > 1.f) {
            throw std::runtime_error("Bgr565Bitmap::from_float_rgb received value > 1");
        }
        return Bgr565{
            (unsigned short)(r * ((1 << 5) - 1) + 0.5f),
            (unsigned short)(g * ((1 << 6) - 1) + 0.5f),
            (unsigned short)(b * ((1 << 5) - 1) + 0.5f)};
    }
} __attribute__((packed));

class Bgr565Bitmap: public Array<Bgr565> {
public:
    Bgr565Bitmap();
    explicit Bgr565Bitmap(const ArrayShape& shape, const Bgr565& color);
    explicit Bgr565Bitmap(const Array<Bgr565>& other);
    explicit Bgr565Bitmap(const ArrayShape& shape);

    void draw_fill_rect(const ArrayShape& center, size_t size, const Bgr565& color);
    void draw_line(const Array<float>& from, const Array<float>& to, size_t thickness, const Bgr565& color);
    void draw_infinite_line(const Array<float>& from, const Array<float>& to, size_t thickness, const Bgr565& color);
    void draw_mask(const Array<bool>& mask, const Bgr565& color);
    void draw_streamline(const ArrayShape& center, const Array<float>& velocity, size_t size, size_t length, const Bgr565& color);

    static Bgr565Bitmap load_from_file(const std::string &filename);
    void save_to_file(const std::string &filename) const;

    static Bgr565Bitmap load_from_stream(std::istream& istream);
    void save_to_stream(std::ostream& ostream) const;

    static Bgr565Bitmap from_float_rgb(const Array<float>& grayscale);
    Array<float> to_float_rgb() const;

    static Bgr565Bitmap from_float_grayscale(const Array<float>& grayscale);
    Array<float> to_float_grayscale() const;
private:
    void draw_line_ext(const Array<float>& from, const Array<float>& to, size_t thickness, const Bgr565& color, bool infinite);
};

}
