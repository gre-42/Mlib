#pragma once
#include <Mlib/Array/Array.hpp>
#include <string>

namespace Mlib {

class Bgr565Bitmap;

// 24-bit, 3 * 8 bytes
struct Bgr24 {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    static inline Bgr24 red() {
        Bgr24 result;
        result.r = 255;
        result.g = 0;
        result.b = 0;
        return result;
    }
    static inline Bgr24 green() {
        Bgr24 result;
        result.r = 0;
        result.g = 255;
        result.b = 0;
        return result;
    }
    static inline Bgr24 blue() {
        Bgr24 result;
        result.r = 0;
        result.g = 0;
        result.b = 255;
        return result;
    }
    Array<float> to_float_rgb() const {
        return Array<float>{
            r / 255.f,
            g / 255.f,
            b / 255.f};
    }
} __attribute__((packed));

class Bgr24Raw: public Array<Bgr24> {
public:
    static Bgr24Raw load_from_file(const std::string &filename);
    static Bgr24Raw load_from_stream(const ArrayShape& shape, std::istream& istream);

    Array<float> to_float_grayscale() const;
    Array<float> to_float_rgb() const;
};

}
