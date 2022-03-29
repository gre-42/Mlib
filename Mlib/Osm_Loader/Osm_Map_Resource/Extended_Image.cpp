#include "Extended_Image.hpp"
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Filters/Box_Filter.hpp>

using namespace Mlib;

ExtendedImage::ExtendedImage(
    const Array<float>& image,
    const Array<bool>& mask,
    size_t extension,
    size_t box_filter_radius,
    size_t niterations,
    bool preserve_original)
: extension_{ extension }
{
    if (extension == 0 && ((box_filter_radius == 0) || preserve_original)) {
        extended_image_ = image;
        return;
    }
    if (image.ndim() != 2) {
        throw std::runtime_error("Dim not 2");
    }
    if (!all(image.shape() == mask.shape())) {
        throw std::runtime_error("Shape mismatch");
    }
    // This check is sufficient for the extension,
    // but not for points in the interior
    if (niterations * box_filter_radius < std::max({ extension, image.shape(0), image.shape(1) })) {
        throw std::runtime_error("niterations * box_filter < extension");
    }
    extended_image_ = nans<float>(image.shape() + 2 * extension);
    for (size_t i = 0; i < niterations; ++i) {
        for (size_t r = 0; r < image.shape(0); ++r) {
            for (size_t c = 0; c < image.shape(1); ++c) {
                if (mask(r, c)) {
                    extended_image_(r + extension_, c + extension_) = image(r, c);
                }
            }
        }
        extended_image_ = box_filter_nans_as_zeros_NWE(extended_image_, ArrayShape{ 2 * box_filter_radius + 1, 2 * box_filter_radius + 1 });
    }
    if (preserve_original) {
        for (size_t r = 0; r < image.shape(0); ++r) {
            for (size_t c = 0; c < image.shape(1); ++c) {
                if (mask(r, c)) {
                    extended_image_(r + extension_, c + extension_) = image(r, c);
                }
            }
        }
    }
    if (any(Mlib::isnan(extended_image_))) {
        throw std::runtime_error("Extended image contains NAN values");
    }
}

bool ExtendedImage::operator () (float r, float c, float& value) const {
    return bilinear_grayscale_interpolation(r + (float)extension_, c + (float)extension_, extended_image_, value);
}
