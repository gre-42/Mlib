#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Bgr565.hpp>
#include <string>

namespace Mlib {

#pragma warning( push )
#pragma warning( disable : 4103 )
#include <Mlib/Packed_Begin.hpp>
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
} PACKED;
#include <Mlib/Packed_End.hpp>
#pragma warning( pop )

static const unsigned char off_bitmap_header_565[] = {
    0, 0xf8, 0, 0, 0xe0, 0x7, 0, 0, 0x1f, 0, 0, 0, 0, 0, 0, 0,
    0x42, 0x47, 0x52, 0x73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0};

class Bgr565Bitmap: public Array<Bgr565> {
public:
    Bgr565Bitmap();
    explicit Bgr565Bitmap(const ArrayShape& shape, const Bgr565& color);
    explicit Bgr565Bitmap(const Array<Bgr565>& other);
    explicit Bgr565Bitmap(const ArrayShape& shape);

    void draw_fill_rect(const FixedArray<size_t, 2>& center, size_t size, const Bgr565& color);
    void draw_empty_rect(const FixedArray<size_t, 2>& center, size_t size, const Bgr565& color);
    void draw_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Bgr565& color);
    void draw_infinite_line(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Bgr565& color);
    void draw_mask(const Array<bool>& mask, const Bgr565& color);
    void draw_streamline(const FixedArray<size_t, 2>& center, const Array<float>& velocity, size_t size, size_t length, const Bgr565& color);

    static Bgr565Bitmap load_from_file(const std::string &filename);
    void save_to_file(const std::string &filename) const;

    static Bgr565Bitmap load_from_stream(std::istream& istream);
    void save_to_stream(std::ostream& ostream) const;

    static Bgr565Bitmap from_float_rgb(const Array<float>& grayscale);
    Array<float> to_float_rgb() const;

    static Bgr565Bitmap from_float_grayscale(const Array<float>& grayscale);
    Array<float> to_float_grayscale() const;
private:
    void draw_line_ext(const FixedArray<float, 2>& from, const FixedArray<float, 2>& to, size_t thickness, const Bgr565& color, bool infinite);
};

}
