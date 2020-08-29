#pragma once
#include <Mlib/Array/Array.hpp>
#include <string>

namespace Mlib {

class PgmImage;

static const uint16_t uint16_nan = (1 << 8);

static inline uint16_t uint16_from_float(float grayscale) {
    if (std::isnan(grayscale)) {
        return uint16_nan;
    }
    // consider using grayscale.clip(0, 1) if this fails
    if (grayscale < 0) {
        throw std::runtime_error("PgmImage::from_float received " + std::to_string(grayscale) + "<0");
    }
    if (grayscale > 1) {
        throw std::runtime_error("PgmImage::from_float received " + std::to_string(grayscale) + ">1");
    }
    return (uint16_t)(grayscale * UINT16_MAX + 0.5f);
}

class PgmImage: public Array<uint16_t> {
public:
    PgmImage();
    explicit PgmImage(const ArrayShape& shape, uint16_t color);
    explicit PgmImage(const Array<uint16_t>& other);
    explicit PgmImage(const ArrayShape& shape);

    void draw_fill_rect(const ArrayShape& center, size_t size, uint16_t color);
    void draw_line(const Array<float>& from, const Array<float>& to, size_t thickness, uint16_t color);
    void draw_infinite_line(const Array<float>& from, const Array<float>& to, size_t thickness, uint16_t color);
    void draw_mask(const Array<bool>& mask, uint16_t color);
    void draw_streamline(const ArrayShape& center, const Array<float>& velocity, size_t size, size_t length, uint16_t color);

    static PgmImage load_from_file(const std::string& filename);
    static PgmImage load_from_stream(std::istream& istream);

    void save_to_file(const std::string &filename) const;
    void save_to_stream(std::ostream& ostream) const;

    static PgmImage from_float(const Array<float>& grayscale);

    Array<float> to_float() const;
private:
    void draw_line_ext(const Array<float>& from, const Array<float>& to, size_t thickness, uint16_t color, bool infinite);
};

}
