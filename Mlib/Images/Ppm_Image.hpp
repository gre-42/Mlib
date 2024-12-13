#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Rgb24.hpp>
#include <string>

namespace Mlib {

class PpmImage;

class PpmImage: public Array<Rgb24> {
public:
    PpmImage();
    ~PpmImage();
    explicit PpmImage(const ArrayShape& shape, const Rgb24& color);
    explicit PpmImage(const Array<Rgb24>& other);
    explicit PpmImage(const ArrayShape& shape);

    PpmImage T() const;
    PpmImage reversed(size_t axis) const;

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgb24& color);
    void draw_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_infinite_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgb24& color, const Rgb24* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, const Rgb24& color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, const Rgb24& color);

    static PpmImage load_from_file(const std::string& filename);
    static PpmImage load_from_stream(std::istream& istream);

    void save_to_file(const std::string &filename) const;
    void save_to_stream(std::ostream& ostream) const;

    static PpmImage from_float_rgb(const Array<float>& grayscale);
    static PpmImage from_float_grayscale(const Array<float>& grayscale);

    Array<float> to_float_grayscale() const;
    Array<float> to_float_rgb() const;
private:
    void draw_line_ext(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgb24& color, bool infinite, const Rgb24* short_line_color = nullptr);
};

}
