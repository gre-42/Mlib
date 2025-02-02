#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Rgb24.hpp>
#include <cstdint>
#include <string>

template <class TData>
struct StbInfo;

namespace Mlib {

class StbImage3: public Array<Rgb24> {
public:
    StbImage3();
    ~StbImage3();
    explicit StbImage3(const FixedArray<size_t, 2>& shape, const Rgb24& color);
    explicit StbImage3(const Array<Rgb24>& other);
    explicit StbImage3(const FixedArray<size_t, 2>& shape);
    explicit StbImage3(const StbInfo<uint8_t>& stb_info);

    StbImage3 T() const;
    StbImage3 reversed(size_t axis) const;

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color);
    void draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color);
    void draw_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_infinite_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, const Rgb24& color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, const Rgb24& color);

    static StbImage3 load_from_file(const std::string& filename);

    void save_to_file(const std::string &filename, int jpg_quality = 95) const;

    static StbImage3 from_rgb(const Array<uint8_t>& grayscale);
    static StbImage3 from_float_rgb(const Array<float>& grayscale);
    static StbImage3 from_float_grayscale(const Array<float>& grayscale);

    Array<float> to_float_grayscale() const;
    Array<float> to_float_rgb() const;
private:
    void draw_line_ext(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgb24& color, bool infinite, const Rgb24* short_line_color = nullptr);
};

}
