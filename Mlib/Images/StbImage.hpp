#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Rgb24.hpp>
#include <string>

namespace Mlib {

class StbImage: public Array<Rgb24> {
public:
    StbImage();
    explicit StbImage(const ArrayShape& shape, const Rgb24& color);
    explicit StbImage(const Array<Rgb24>& other);
    explicit StbImage(const ArrayShape& shape);

    StbImage T() const;
    StbImage reversed(size_t axis) const;

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color);
    void draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color);
    void draw_line(const Array<float>& from, const Array<float>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_infinite_line(const Array<float>& from, const Array<float>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, const Rgb24& color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, const Rgb24& color);

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
