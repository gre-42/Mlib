#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Assert.hpp>
#include <stb_cpp/stb_image_load.hpp>

namespace Mlib {

template <class TData>
Array<TData> stb_image_2_array(const TData* data, int width, int height, int nrChannels) {
    Array<TData> result{ArrayShape{(size_t)nrChannels, (size_t)height, (size_t)width}};
    for (size_t r = 0; r < (size_t)height; ++r) {
        for (size_t c = 0; c < (size_t)width; ++c) {
            for (size_t d = 0; d < (size_t)nrChannels; ++d) {
                result(d, r, c) = data[(r * (size_t)width  + c) * (size_t)nrChannels + d];
            }
        }
    }
    return result;
}

template <class TData>
void array_2_stb_image(const Array<TData>& array, TData* data) {
    assert_true(array.ndim() == 3);
    for (size_t d = 0; d < array.shape(0); ++d) {
        for (size_t r = 0; r < array.shape(1); ++r) {
            for (size_t c = 0; c < array.shape(2); ++c) {
                data[(r * array.shape(2)  + c) * array.shape(0) + d] = array(d, r, c);
            }
        }
    }
}

template <class TData>
Array<TData> stb_image_2_array(const StbInfo<TData>& image) {
    return stb_image_2_array(image.data(), image.width, image.height, image.nrChannels);
}

}
