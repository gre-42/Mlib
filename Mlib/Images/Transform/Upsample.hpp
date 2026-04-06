#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <stdexcept>

namespace Mlib {

template <class T>
class UpsampledPixel {
public:
    UpsampledPixel(
        const Array<T>& image,
        const T& rf,
        const T& cf)
        : image_{image}
    {
        if (image.ndim() != 3) {
            throw std::runtime_error("Image does not have 3 dimensions");
        }
        if (!bilinear_interpolation(rf, cf, image.shape(1), image.shape(2), bi_)) {
            throw std::runtime_error("Could not interpolate");
        }
    }
    T operator [] (size_t channel) const {
        return bi_(image_, channel);
    }
private:
    const Array<T>& image_;
    BilinearInterpolator<T> bi_;
};

template <class T>
class Upsample {
public:
    Upsample(const Array<T>& image, size_t width, size_t height)
        : image_(image)
    {
        if (image_.ndim() != 3) {
            throw std::runtime_error("Image does not have 3 dimensions");
        }
        if (image_.shape(2) > width) {
            throw std::runtime_error("Image width too large");
        }
        if (image_.shape(1) > height) {
            throw std::runtime_error("Image height too large");
        }
        fac_width_ = integral_to_float<T>(image_.shape(2)) / integral_to_float<T>(width);
        fac_height_ = integral_to_float<T>(image_.shape(1)) / integral_to_float<T>(height);
    }
    UpsampledPixel<T> operator () (size_t r, size_t c) const {
        return {image_, integral_to_float<T>(r) * fac_height_, integral_to_float<T>(c) * fac_width_};
    }
private:
    Array<T> image_;
    T fac_width_;
    T fac_height_;
};

}
