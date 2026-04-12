#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Ia16.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <cstdint>
#include <string>

template <class TData>
class StbInfo;

namespace Mlib {

class StbImage2: public Array<Ia16> {
public:
    StbImage2();
    ~StbImage2();
    explicit StbImage2(const FixedArray<size_t, 2>& shape, const Ia16& color);
    explicit StbImage2(const Array<Ia16>& other);
    explicit StbImage2(const FixedArray<size_t, 2>& shape);
    explicit StbImage2(const StbInfo<uint8_t>& stb_info);

    StbImage2 T() const;
    StbImage2 reversed(size_t axis) const;

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Ia16& color);
    void draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Ia16& color);
    void draw_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Ia16& color, const Ia16* short_line_color = nullptr);
    void draw_infinite_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Ia16& color, const Ia16* short_line_color = nullptr);
    void draw_mask(const Array<bool>& mask, const Ia16& color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, const Ia16& color);

    static StbImage2 load_from_file(const Utf8Path& filename);

    void save_to_file(const Utf8Path& filename, int jpg_quality = 95) const;

    static StbImage2 from_ia(const Array<uint8_t>& grayscale);
    static StbImage2 from_float_ia(const Array<float>& grayscale);

    Array<float> to_float_ia() const;
private:
    void draw_line_ext(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Ia16& color, bool infinite, const Ia16* short_line_color = nullptr);
};

}
