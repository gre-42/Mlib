#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template <class TData>
Array<TData> to_multichannel_2d(const Array<TData>& image) {
    switch (image.ndim()) {
        case 2: return image.reshaped(ArrayShape{1}.concatenated(image.shape()));
        case 3: return image;
    }
    throw std::runtime_error("Unexpected image dimensionality");
}

template <class TData>
Array<TData> to_singlechannel_2d(const Array<TData>& image) {
    switch (image.ndim()) {
        case 2: return image;
        case 3:
            switch (image.shape(0)) {
                case 0: throw std::runtime_error("Unexpected number of channels");
                case 1: return image[0];
                default: return image;
            }
    }
    throw std::runtime_error("Unexpected image dimensionality");
}

}