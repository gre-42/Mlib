#include "Corresponding_Features_In_Box.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

CorrespondingFeaturesInBox::CorrespondingFeaturesInBox(
    const Array<float>& feature_points0,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb)
{
    const size_t max_distance = 15;
    const float worst_error = 1.f; // Errors respect the patch-brightness.

    std::list<Array<float>> yl0_2;
    std::list<Array<float>> yl1_2;
    std::list<Array<float>> yl0;
    std::list<Array<float>> yl1;
    for (const Array<float>& f : feature_points0) {
        TraceablePatch tp{im0_rgb, a2i(f), ArrayShape{10, 10}};
        if (tp.good_) {
            ArrayShape new_pos = tp.new_position_in_box(im1_rgb, a2i(f), ArrayShape{2 * max_distance + 1, 2 * max_distance + 1}, worst_error);
            if (new_pos(0) < 1e6) {
                yl0_2.push_back(f);
                yl1_2.push_back(i2a(new_pos));
            }
        }
    }
    y0_2d = Array<float>{yl0_2};
    y1_2d = Array<float>{yl1_2};
}
