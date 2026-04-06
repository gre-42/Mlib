
#include "Match_Rgba_Histograms.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Histogram_Matching.hpp>

using namespace Mlib;

template <class T>
HistogramsAndMatched<T, T, float> match_rgba_histograms_generic(
    const Array<T>& image,
    const Array<T>& ref,
    T alpha_threshold)
{
    if (image.ndim() != 3) {
        throw std::runtime_error("Image does not have 3 dimensions");
    }
    if (ref.ndim() != 3) {
        throw std::runtime_error("Reference image does not have 3 dimensions");
    }
    auto nc = [](const Array<T>& im) -> size_t {
        switch (im.shape(0)) {
            case 1: return 1;
            case 2: return 1;
            case 3: return 3;
            case 4: return 3;
        }
        throw std::runtime_error("Unsupported number of channels");
    };
    auto nci = nc(image);
    auto ncr = nc(ref);
    if (nci != ncr) {
        throw std::runtime_error("Unsupported combination of channels");
    }
    auto c3 = [nci]() -> size_t {
        switch (nci) {
            case 1: return 1;
            case 3: return 3;
        }
        throw std::runtime_error("Unexpected number of colors");
    }();
    auto c4 = c3 + 1;
    HistogramsAndMatched<T, T, float> result;
    result.matched = Array<T>{image.shape()};
    result.hms.reserve(image.shape(0));
    if ((image.shape(0) == c3) && (ref.shape(0) == c3)) {
        for (size_t d = 0; d < c3; ++d) {
            auto hpm = HistogramAndMatched<T, T, float>{image[d], ref[d], 256, OutOfBoundsBehavior::CLAMP};
            result.matched[d] = hpm.matched;
            result.hms.emplace_back(std::move(hpm.hm));
        }
    } else {
        Array<bool> mask = (image.shape(0) == c3)
            ? ones<bool>(image.shape().erased_first())
            : (image[c3] > alpha_threshold);
        Array<bool> mask_ref = (ref.shape(0) == c3)
            ? ones<bool>(ref.shape().erased_first())
            : (ref[c3] > alpha_threshold);
        for (size_t d = 0; d < c3; ++d) {
            HistogramMatching<T, T, float> hm{image[d][mask], ref[d][mask_ref], 256};
            for (size_t r = 0; r < image.shape(1); ++r) {
                for (size_t c = 0; c < image.shape(2); ++c) {
                    result.matched(d, r, c) = hm(image(d, r, c), OutOfBoundsBehavior::CLAMP);
                }
            }
            result.hms.emplace_back(std::move(hm));
        }
        if (image.shape(0) == c4) {
            result.matched[c3] = image[c3];
        }
    }
    return result;
}

HistogramsAndMatched<unsigned char, unsigned char, float> Mlib::match_rgba_histograms(
    const Array<unsigned char>& image,
    const Array<unsigned char>& ref,
    unsigned char alpha_threshold)
{
    return match_rgba_histograms_generic(image, ref, alpha_threshold);
}

HistogramsAndMatched<float, float, float> Mlib::match_rgba_histograms(
    const Array<float>& image,
    const Array<float>& ref,
    float alpha_threshold)
{
    return match_rgba_histograms_generic(image, ref, alpha_threshold);
}
