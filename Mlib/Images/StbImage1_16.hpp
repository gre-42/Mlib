#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/I16.hpp>
#include <cmath>
#include <string>

template <class TData>
class StbInfo;

namespace Mlib {

class StbImage1_16: public Array<uint16_t> {
public:
    StbImage1_16();
    ~StbImage1_16();
    explicit StbImage1_16(const ArrayShape& shape, uint16_t color);
    explicit StbImage1_16(const Array<uint16_t>& other);
    explicit StbImage1_16(const ArrayShape& shape);
    explicit StbImage1_16(const StbInfo<uint16_t>& stb_info);

    StbImage1_16 T() const;
    StbImage1_16 reversed(size_t axis) const;

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, uint16_t color);
    void draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, uint16_t color);
    void draw_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, uint16_t color, const uint16_t* short_line_color = nullptr);
    void draw_infinite_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, uint16_t color, const uint16_t* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, uint16_t color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, uint16_t color);

    static StbImage1_16 load_from_file(const std::string& filename);

    void save_to_file(const std::string &filename, int jpg_quality = 95) const;

    static StbImage1_16 from_float_grayscale(const Array<float>& grayscale);

    Array<float> to_float_grayscale() const;
private:
    void draw_line_ext(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, uint16_t color, bool infinite, const uint16_t* short_line_color = nullptr);
};

}
