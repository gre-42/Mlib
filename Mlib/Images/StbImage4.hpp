#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Rgba32.hpp>
#include <cstdint>
#include <string>

template <class TData>
struct StbInfo;

namespace Mlib {

class StbImage3;

class StbImage4: public Array<Rgba32> {
public:
    StbImage4();
    ~StbImage4();
    explicit StbImage4(const ArrayShape& shape, const Rgba32& color);
    explicit StbImage4(const Array<Rgba32>& other);
    explicit StbImage4(const ArrayShape& shape);
    explicit StbImage4(const StbInfo<uint8_t>& stb_info);

    StbImage4 T() const;
    StbImage4 reversed(size_t axis) const;

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgba32& color);
    void draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Rgba32& color);
    void draw_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgba32& color, const Rgba32* short_line_color = nullptr);
    void draw_infinite_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgba32& color, const Rgba32* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, const Rgba32& color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, const Rgba32& color);

    static StbImage4 load_from_file(const std::string& filename);

    void save_to_file(const std::string &filename, int jpg_quality = 95) const;

    static StbImage4 from_float_rgba(const Array<float>& rgba);

    Array<float> to_float_rgba() const;

    StbImage3 to_rgb() const;
private:
    void draw_line_ext(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Rgba32& color, bool infinite, const Rgba32* short_line_color = nullptr);
};

}
