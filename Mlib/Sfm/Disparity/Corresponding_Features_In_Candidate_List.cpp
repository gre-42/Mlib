#include "Corresponding_Features_In_Candidate_List.hpp"
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

CorrespondingFeaturesInCandidateList::CorrespondingFeaturesInCandidateList(
    const Array<FixedArray<float, 2>>& feature_points0,
    const Array<FixedArray<float, 2>>& feature_points1,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    size_t patch_size)
{
    const float worst_error = 1.f; // Errors respect the patch-brightness.

    std::list<FixedArray<float, 2>> yl0;
    std::list<FixedArray<float, 2>> yl1;
    for (const FixedArray<float, 2>& f : feature_points0.flat_iterable()) {
        TraceablePatch tp{im0_rgb, a2i(f), FixedArray<size_t, 2>{ patch_size, patch_size }};
        if (tp.good_) {
            FixedArray<float, 2> new_pos = tp.new_position_in_candidate_list(im1_rgb, f, feature_points1, worst_error);
            if (new_pos(0) < 1e6) {
                yl0.push_back(f);
                yl1.push_back(new_pos);
            }
        }
    }
    y0 = Array<FixedArray<float, 2>>{yl0};
    y1 = Array<FixedArray<float, 2>>{yl1};
}
