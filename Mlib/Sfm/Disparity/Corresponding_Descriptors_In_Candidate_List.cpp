#include "Corresponding_Descriptors_In_Candidate_List.hpp"
#include <Mlib/Images/Coordinates_Fixed.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

CorrespondingDescriptorsInCandidateList::CorrespondingDescriptorsInCandidateList(
    const Array<FixedArray<float, 2>>& feature_points0,
    const Array<FixedArray<float, 2>>& feature_points1,
    const Array<float>& descriptors0,
    const Array<float>& descriptors1,
    float lowe_ratio)
{
    assert(descriptors0.ndim() == 2);
    assert(descriptors1.ndim() == 2);
    assert(feature_points0.length() == descriptors0.shape(0));
    assert(feature_points1.length() == descriptors1.shape(0));

    std::list<FixedArray<float, 2>> yl0_2;
    std::list<FixedArray<float, 2>> yl1_2;
    std::list<Array<float>> yl0;
    std::list<Array<float>> yl1;
    for (size_t i0 = 0; i0 < feature_points0.length(); ++i0) {
        size_t best_i1 = SIZE_MAX;
        float best_dist = INFINITY;
        float second_best_dist = INFINITY;
        for (size_t i1 = 0; i1 < feature_points1.length(); ++i1) {
            float dist = 0;
            for (size_t d = 0; d < descriptors0.shape(1); ++d) {
                dist += squared(descriptors0(i0, d) - descriptors1(i1, d));
            }
            if (dist < best_dist) {
                best_i1 = i1;
                second_best_dist = best_dist;
                best_dist = dist;
            }
        }
        if (best_dist < squared(lowe_ratio) * second_best_dist) {
            yl0_2.push_back(feature_points0(i0));
            yl1_2.push_back(feature_points1(best_i1));
        }
    }
    y0_2d = Array<FixedArray<float, 2>>{yl0_2};
    y1_2d = Array<FixedArray<float, 2>>{yl1_2};
}
