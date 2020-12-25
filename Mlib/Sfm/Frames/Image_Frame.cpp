#include "Image_Frame.hpp"
#include <Mlib/Images/Bgr565Bitmap.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

ImageFrame::ImageFrame() {}

ImageFrame& ImageFrame::operator = (float rhs) {
    grayscale = rhs;
    mask = ones<bool>(grayscale.shape());
    return *this;
}

void ImageFrame::load_from_file(const std::string& filename) {
    auto bmp = Bgr565Bitmap::load_from_file(filename);
    grayscale = bmp.to_float_grayscale();
}

void ImageFrame::save_to_file(const std::string& filename) const {
    if (grayscale.ndim() != 2) {
        throw std::runtime_error("ImageFrame::save_to_file: ndim!=2, but " + grayscale.shape().str());
    }
    auto bmp = Bgr565Bitmap::from_float_grayscale(grayscale);
    bmp.save_to_file(filename);
}

void ImageFrame::save_axis_to_file(const std::string& filename, size_t axis, float min, float max) const {
    if (grayscale.ndim() != 3) {
        throw std::runtime_error("ImageFrame::save_to_file: ndim!=3, shape=" + grayscale.shape().str());
    }
    assert(axis < grayscale.shape(0));
    assert(all(mask.shape() == grayscale[axis].shape()));
    Bgr565Bitmap bmp(mask.shape());
    auto f = grayscale[axis].flattened();
    auto m = mask.flattened();
    auto b = bmp.flattened();
    for (size_t j = 0; j < m.length(); ++j) {
        if (m(j)) {
            b(j) = Bgr565::from_float_grayscale((f(j) - min) / (max - min));
        } else {
            b(j) = Bgr565::red();
        }
    }
    bmp.save_to_file(filename);
}

Array<float> ImageFrame::grayscale_x3() const {
    if (!grayscale_x3_.initialized()) {
        grayscale_x3_.resize(ArrayShape{rgb.shape()});
        assert_true(rgb.shape(0) == 3);
        grayscale_x3_[0] = grayscale;
        grayscale_x3_[1] = gaussian_filter_NWE(grayscale, 1.f, NAN);
        grayscale_x3_[2] = gaussian_filter_NWE(grayscale, 2.f, NAN);
    }
    return grayscale_x3_;
}
