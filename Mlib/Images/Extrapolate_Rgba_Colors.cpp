
#include "Extrapolate_Rgba_Colors.hpp"
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <stdexcept>

using namespace Mlib;

Array<float> Mlib::extrapolate_rgba_colors(const Array<float>& source, float sigma, size_t niterations) {
    if (source.ndim() != 3) {
        throw std::runtime_error("Image does not have 3 dimensions");
    }
    if (source.shape(0) != 4) {
        throw std::runtime_error("Image does not have 4 channels");
    }
    auto destination = source.copy();
    
    for (size_t i = 0; i < niterations; ++i) {
        Array<float> m =
            gaussian_filter_NWE(destination[3], sigma, float{NAN})
            .applied([](float v){return v == 0 ? float{NAN} : v;});
        for (size_t d = 0; d < 3; ++d) {
            TemporarilyIgnoreFloatingPointExeptions ignore_except;
            destination[d] = gaussian_filter_NWE(destination[d] * destination[3], sigma, float{NAN}) / m;

            auto d1 = destination[d].flattened();
            auto s1 = source[d].flattened();
            auto a1 = source[3].flattened();
            for (size_t j = 0; j < d1.length(); ++j) {
                if (a1(j) > 1e-1) {
                    d1(j) = s1(j);
                }
            }
        }
        destination[3] = m;
        if (!any(Mlib::isnan(m))) {
            break;
        }
    }
    destination[3] = source[3];
    
    return destination;
}

StbImage4 Mlib::extrapolate_rgba_colors(const StbImage4& img, float sigma, size_t niterations) {
    auto destination = extrapolate_rgba_colors(img.to_float_rgba(), sigma, niterations);
    return StbImage4::from_float_rgba(clipped(destination, 0.f, 1.f));
}
