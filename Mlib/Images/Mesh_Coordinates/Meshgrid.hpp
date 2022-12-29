#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstddef>

namespace Mlib {

template <class TImage>
void meshgrid(TImage& image, size_t axis) {
    typedef typename TImage::value_type TValue;
    if (image.ndim() == 1) {
        if (axis > 0) {
            THROW_OR_ABORT("Axis out of bounds");
        }
        for (size_t i = 0; i < image.length(); ++i) {
            image(i) = (TValue)i;
        }
    } else if (image.ndim() == 2) {
        if (axis > 1) {
            THROW_OR_ABORT("Axis out of bounds");
        }
        size_t i[2];
        for (i[0] = 0; i[0] < image.shape(0); ++i[0]) {
            for (i[1] = 0; i[1] < image.shape(1); ++i[1]) {
                image(i[0], i[1]) = (TValue)i[axis];
            }
        }
    } else {
        THROW_OR_ABORT("Unsupported meshgrid");
    }
}

}
