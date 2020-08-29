#include "Optical_Flow.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

void Mlib::optical_flow(
    const Array<float>& image0,
    const Array<float>& image1,
    const Array<float>* image2,
    const ArrayShape& window_shape,
    float max_displacement,
    Array<float>& flow,
    Array<bool>& mask)
{
    assert(image0.ndim() == 2);
    assert(all(image0.shape() == image1.shape()));
    if (image2 != nullptr) {
        assert(all(image0.shape() == image2->shape()));
    }
    Array<float> It = image2 != nullptr
        ? (*image2 - image0) / 2.f
        : image1 - image0;
    Array<float> Is = image2 != nullptr
        ? image1
        : (image0 + image1) / 2.f;
    Array<float> Ix = difference_filter_1d(Is, NAN, id1);
    Array<float> Iy = difference_filter_1d(Is, NAN, id0);
    auto a = box_filter_nan(Ix * Ix, window_shape, NAN);
    auto b = box_filter_nan(Ix * Iy, window_shape, NAN);
    // box_filter_nan(Iy * Ix, window_shape); // c == b
    auto d = box_filter_nan(Iy * Iy, window_shape, NAN);
    auto rdetM = 1.f / (a * d - b * b);
    mask.do_resize(image0.shape());
    auto rdetM1D = rdetM.flattened();
    auto mask1D = mask.flattened();
    // mask to avoid floating-point exception (0 * infty)
    for(size_t i = 0; i < rdetM1D.length(); ++i) {
        mask1D(i) = (std::abs(rdetM1D(i)) < 1e12);
        if (!mask1D(i)) {
            rdetM1D(i) = std::numeric_limits<float>::quiet_NaN();
        }
    }
    Array<float> B({
        -box_filter_nan(Ix * It, window_shape, NAN),
        -box_filter_nan(Iy * It, window_shape, NAN)});
    flow = Array<float>({
        rdetM * (d * B[0] - b * B[1]),
        rdetM * (a * B[1] - b * B[0]),
    });
    for(size_t axis = 0; axis < flow.shape(0); ++axis) {
        auto f = flow[axis].flattened();
        // this also masks the quiet NaNs
        mask1D &= (abs(f) < max_displacement);
    }
}
