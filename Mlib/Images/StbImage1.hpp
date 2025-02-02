#pragma once
#include <Mlib/Array/Array.hpp>
#include <string>

template <class TData>
struct StbInfo;

namespace Mlib {

static const uint8_t uint8_nan = (1 << 4);

static inline uint8_t uint8_from_float(float grayscale) {
    if (std::isnan(grayscale)) {
        return uint8_nan;
    }
    // consider using grayscale.clip(0, 1) if this fails
    if (grayscale < 0) {
        THROW_OR_ABORT("PgmImage::from_float received " + std::to_string(grayscale) + "<0");
    }
    if (grayscale > 1) {
        THROW_OR_ABORT("PgmImage::from_float received " + std::to_string(grayscale) + ">1");
    }
    return (uint8_t)(grayscale * UINT8_MAX + 0.5f);
}

class StbImage1: public Array<uint8_t> {
public:
    StbImage1();
    ~StbImage1();
    explicit StbImage1(const ArrayShape& shape, uint8_t color);
    explicit StbImage1(const Array<uint8_t>& other);
    explicit StbImage1(const ArrayShape& shape);
    explicit StbImage1(const StbInfo<uint8_t>& stb_info);

    StbImage1 T() const;
    StbImage1 reversed(size_t axis) const;

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, uint8_t color);
    void draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, uint8_t color);
    void draw_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, uint8_t color, const uint8_t* short_line_color = nullptr);
    void draw_infinite_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, uint8_t color, const uint8_t* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, uint8_t color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, uint8_t color);

    static StbImage1 load_from_file(const std::string& filename);

    void save_to_file(const std::string &filename, int jpg_quality = 95) const;

    static StbImage1 from_float_grayscale(const Array<float>& grayscale);

    Array<float> to_float_grayscale() const;
private:
    void draw_line_ext(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, uint8_t color, bool infinite, const uint8_t* short_line_color = nullptr);
};

}
