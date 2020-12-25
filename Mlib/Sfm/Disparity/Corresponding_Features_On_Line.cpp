#include "Corresponding_Features_On_Line.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Sfm/Disparity/Epiline_Direction.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

/**
 * Cf. "compute_disparity"
 */
CorrespondingFeaturesOnLine::CorrespondingFeaturesOnLine(
    const Array<float>& feature_points0,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const Array<float>& F)
{
    const size_t max_distance = 200;
    const size_t worst_error = 1.f; // Errors respect the patch-brightness.

    std::list<Array<float>> yl0_2;
    std::list<Array<float>> yl1_2;
    std::list<Array<float>> yl0;
    std::list<Array<float>> yl1;
    for (const Array<float>& f0 : feature_points0) {
        ArrayShape id = a2i(f0);
        EpilineDirection ed(id(0), id(1), F);
        if (ed.good) {
            TraceablePatch tp{im0_rgb, id, ArrayShape{10, 10}};
            if (tp.good_) {
                float new_pos = tp.new_position_on_line(im1_rgb, a2i(ed.center1), ed.v1, max_distance, worst_error);
                if (!std::isnan(new_pos)) {
                    yl0_2.push_back(f0);
                    yl1_2.push_back(ed.center1 + new_pos * ed.v1);
                }
            }
        }
    }
    y0_2d = Array<float>{yl0_2};
    y1_2d = Array<float>{yl1_2};
}
