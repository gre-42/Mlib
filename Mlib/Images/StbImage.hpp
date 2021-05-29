#pragma once
#include <Mlib/Array/Array.hpp>
#include <string>

namespace Mlib {

class StbImage;

#include <Mlib/Packed_Begin.hpp>
// 24-bit, 3 * 8 bytes
struct Rgb24 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    Rgb24() {}
    Rgb24(
        unsigned char r,
        unsigned char g,
        unsigned char b)
    :r(r), g(g), b(b) {}
    static inline Rgb24 red() {
        return Rgb24{255, 0, 0};
    }
    static inline Rgb24 green() {
        return Rgb24{0, 255, 0};
    }
    static inline Rgb24 blue() {
        return Rgb24{0, 0, 255};
    }
    static inline Rgb24 black() {
        return Rgb24{0, 0, 0};
    }
    static inline Rgb24 white() {
        return Rgb24{255, 255, 255};
    }
    static inline Rgb24 nan() {
        return Rgb24{255, 0, 255};
    }
    static inline Rgb24 gray() {
        return Rgb24{127, 127, 127};
    }
    static inline Rgb24 yellow() {
        return Rgb24{255, 255, 0};
    }
    static inline Rgb24 orange() {
        return Rgb24{255, 127, 0};
    }
    static inline Rgb24 from_float_grayscale(float grayscale) {
        if (std::isnan(grayscale)) {
            return Rgb24::nan();
        }
        // consider using grayscale.clip(0, 1) if this fails
        if (grayscale < 0) {
            throw std::runtime_error("StbImage::from_float_grayscale received " + std::to_string(grayscale) + "<0");
        }
        if (grayscale > 1) {
            throw std::runtime_error("StbImage::from_float_grayscale received " + std::to_string(grayscale) + ">1");
        }
        return Rgb24{
            (unsigned char)(grayscale * 255 + 0.5f),
            (unsigned char)(grayscale * 255 + 0.5f),
            (unsigned char)(grayscale * 255 + 0.5f)};
    }
    static inline Rgb24 from_float_rgb(float r, float g, float b) {
        if (std::isnan(r) || std::isnan(g) || std::isnan(b)) {
            if (!std::isnan(r) || !std::isnan(g) || !std::isnan(b)) {
                throw std::runtime_error("StbImage::from_float_rgb received inconsistent NANs");
            }
            return Rgb24::nan();
        }
        if (r < 0.f || g < 0.f || b < 0.f) {
            throw std::runtime_error("StbImage::from_float_rgb received value < 0");
        }
        if (r > 1.f || g > 1.f || b > 1.f) {
            throw std::runtime_error("StbImage::from_float_rgb received value > 1");
        }
        return Rgb24{
            (unsigned char)(r * 255 + 0.5f),
            (unsigned char)(g * 255 + 0.5f),
            (unsigned char)(b * 255 + 0.5f)};
    }
} PACKED;
#include <Mlib/Packed_End.hpp>

class StbImage: public Array<Rgb24> {
public:
    StbImage();
    explicit StbImage(const ArrayShape& shape, const Rgb24& color);
    explicit StbImage(const Array<Rgb24>& other);
    explicit StbImage(const ArrayShape& shape);

    StbImage T() const;
    StbImage reversed(size_t axis) const;

    void draw_fill_rect(const ArrayShape& center, size_t size, const Rgb24& color);
    void draw_line(const Array<float>& from, const Array<float>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_infinite_line(const Array<float>& from, const Array<float>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, const Rgb24& color);
    void draw_streamline(const ArrayShape& center, const Array<float>& velocity, size_t size, size_t length, const Rgb24& color);

    static StbImage load_from_file(const std::string& filename);
    static StbImage load_from_stream(std::istream& istream);

    void save_to_file(const std::string &filename) const;
    void save_to_stream(std::ostream& ostream) const;

    static StbImage from_float_rgb(const Array<float>& grayscale);
    static StbImage from_float_grayscale(const Array<float>& grayscale);

    Array<float> to_float_grayscale() const;
    Array<float> to_float_rgb() const;
private:
    void draw_line_ext(const Array<float>& from, const Array<float>& to, size_t thickness, const Rgb24& color, bool infinite, const Rgb24* short_line_color = nullptr);
};

}
