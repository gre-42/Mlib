#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

class ExtendedImage {
public:
    ExtendedImage(
        const Array<float>& image,
        const Array<bool>& mask,
        size_t extension,
        size_t box_filter_radius,
        size_t niterations);
    bool operator () (float r, float c, float& value) const;
private:
    Array<float> extended_image_;
    size_t extension_;
};

}
