#include "Match_Rgba_Histograms.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Histogram_Matching.hpp>

using namespace Mlib;

Array<unsigned char> Mlib::match_rgba_histograms(
    const Array<unsigned char>& image,
    const Array<unsigned char>& ref,
    unsigned char alpha_threshold)
{
    Array<unsigned char> out{image.shape()};
    if (image.ndim() != 3) {
        throw std::runtime_error("Image does not have 3 dimensions");
    }
    if (ref.ndim() != 3) {
        throw std::runtime_error("Reference image does not have 3 dimensions");
    }
    if (image.shape(0) != 3 && image.shape(0) != 4) {
        throw std::runtime_error("Image does not have 3 or 4 channels");
    }
    if (ref.shape(0) != 3 && ref.shape(0) != 4) {
        throw std::runtime_error("Reference image does not have 3 or 4 channels");
    }
    if (image.shape(0) == 3) {
        for (size_t d = 0; d < 3; ++d) {
            out[d] = histogram_matching<unsigned char, unsigned char, float>(image[d].flattened(), ref[d].flattened(), 256);
        }
    } else if (image.shape(0) == 4) {
        Array<bool> mask = (image[3] > alpha_threshold);
        Array<bool> mask_ref = ref.shape(0) == 3
            ? ones<bool>(ref.shape().erased_first())
            : (ref[3] > alpha_threshold);
        for (size_t d = 0; d < 3; ++d) {
            HistogramMatching<unsigned char, unsigned char, float> hm{image[d][mask], ref[d][mask_ref], 256};
            for (size_t r = 0; r < image.shape(1); ++r) {
                for (size_t c = 0; c < image.shape(2); ++c) {
                    out(d, r, c) = hm(image(d, r, c), true);
                }
            }
        }
        if (image.shape(0) == 4) {
            out[3] = image[3];
        }
    } else {
        throw std::runtime_error("Image does not have 3 or 4 channels");
    }
    out.reshape(image.shape());
    return out;
}
