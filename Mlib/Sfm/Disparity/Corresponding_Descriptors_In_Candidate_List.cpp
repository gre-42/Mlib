#include "Corresponding_Descriptors_In_Candidate_List.hpp"
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Descriptor.hpp>
#include <map>

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

    std::list<FixedArray<float, 2>> yl0;
    std::list<FixedArray<float, 2>> yl1;
    std::map<size_t, size_t> inserted_keypoints1;
    std::list<std::pair<size_t, size_t>> matches;
    for (size_t i0 = 0; i0 < feature_points0.length(); ++i0) {
        size_t best_i1 = TraceableDescriptor::descriptor_id_in_parameter_list(
            &descriptors0(i0, 0),
            descriptors1,
            lowe_ratio);
        if (best_i1 != SIZE_MAX) {
            if ((best_i1 != SIZE_MAX) && !inserted_keypoints1.contains(best_i1)) {
                matches.push_back({ i0, best_i1 });
            }
            ++inserted_keypoints1[best_i1];
        }
    }
    for (const auto& m : matches) {
        if (inserted_keypoints1.at(m.second) == 1) {
            yl0.push_back(feature_points0(m.first));
            yl1.push_back(feature_points1(m.second));
        }
    }
    y0 = Array<FixedArray<float, 2>>{yl0};
    y1 = Array<FixedArray<float, 2>>{yl1};
}
