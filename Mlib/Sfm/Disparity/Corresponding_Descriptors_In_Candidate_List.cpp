#include "Corresponding_Descriptors_In_Candidate_List.hpp"
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Descriptor.hpp>

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
        size_t best_i1 = TraceableDescriptor::descriptor_id_in_parameter_list(
            &descriptors0(i0, 0),
            descriptors1,
            lowe_ratio);
        if (best_i1 != SIZE_MAX) {
            yl0_2.push_back(feature_points0(i0));
            yl1_2.push_back(feature_points1(best_i1));
        }
    }
    y0_2d = Array<FixedArray<float, 2>>{yl0_2};
    y1_2d = Array<FixedArray<float, 2>>{yl1_2};
}
