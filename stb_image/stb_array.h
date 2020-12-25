#pragma once
#include "stb_image_load.h"
#include <Mlib/Array/Array.hpp>

namespace Mlib {

Array<unsigned char> stb_image_2_array(const unsigned char* data, int width, int height, int nrChannels) {
    Array<unsigned char> result{ArrayShape{(size_t)nrChannels, (size_t)height, (size_t)width}};
    for (size_t r = 0; r < (size_t)height; ++r) {
        for (size_t c = 0; c < (size_t)width; ++c) {
            for (size_t d = 0; d < (size_t)nrChannels; ++d) {
                result(d, r, c) = data[(r * width  + c) * nrChannels + d];
            }
        }
    }
    return result;
}

void array_2_stb_image(const Array<unsigned char>& array, unsigned char* data) {
    assert_true(array.ndim() == 3);
    for (size_t d = 0; d < array.shape(0); ++d) {
        for (size_t r = 0; r < array.shape(1); ++r) {
            for (size_t c = 0; c < array.shape(2); ++c) {
                data[(r * array.shape(2)  + c) * array.shape(0) + d] = array(d, r, c);
            }
        }
    }
}

Array<unsigned char> stb_image_2_array(const StbInfo& image) {
    return stb_image_2_array(image.data.get(), image.width, image.height, image.nrChannels);
}

}
