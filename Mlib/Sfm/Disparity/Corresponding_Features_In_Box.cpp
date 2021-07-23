#include "Corresponding_Features_In_Box.hpp"
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

CorrespondingFeaturesInBox::CorrespondingFeaturesInBox(
    const Array<FixedArray<float, 2>>& feature_points0,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    size_t patch_size,
    size_t max_distance)
{
    const float worst_error = 1.f; // Errors respect the patch-brightness.

    std::list<FixedArray<float, 2>> yl0_2;
    std::list<FixedArray<float, 2>> yl1_2;
    std::list<Array<float>> yl0;
    std::list<Array<float>> yl1;
    for (const FixedArray<float, 2>& f : feature_points0.flat_iterable()) {
        TraceablePatch tp{im0_rgb, a2i(f), FixedArray<size_t, 2>{ patch_size, patch_size }};
        if (tp.good_) {
            FixedArray<size_t, 2> new_pos = tp.new_position_in_box(im1_rgb, a2i(f), FixedArray<size_t, 2>{2 * max_distance + 1, 2 * max_distance + 1}, worst_error);
            if (new_pos(0) < 1e6) {
                yl0_2.push_back(f);
                yl1_2.push_back(i2a(new_pos));
            }
        }
    }
    y0_2d = Array<FixedArray<float, 2>>{yl0_2};
    y1_2d = Array<FixedArray<float, 2>>{yl1_2};
}
