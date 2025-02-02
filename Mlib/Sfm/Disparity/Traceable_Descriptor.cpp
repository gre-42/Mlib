#include "Traceable_Descriptor.hpp"

using namespace Mlib::Sfm;

TraceableDescriptor::TraceableDescriptor(const Array<float>& descriptor)
: descriptor_(descriptor)
{}

bool TraceableDescriptor::new_position_in_candidate_list(
    FixedArray<float, 2>& position,
    const Array<float>& feature_points1,
    const Array<float>& descriptors1,
    float lowe_ratio) const
{
    size_t best_id1 = descriptor_id_in_parameter_list(
        descriptor_.flat_begin(),
        descriptors1,
        lowe_ratio);
    if (best_id1 == SIZE_MAX) {
        return false;
    }
    position = feature_points1(best_id1);
    return true;
}

size_t TraceableDescriptor::descriptor_id_in_parameter_list(
    const Array<float>& descriptors1,
    float lowe_ratio)
{
    return descriptor_id_in_parameter_list(
        descriptor_.flat_begin(),
        descriptors1,
        lowe_ratio);
}
    
size_t TraceableDescriptor::descriptor_id_in_parameter_list(
    const float* descriptor0,
    const Array<float>& descriptors1,
    float lowe_ratio)
{
    size_t best_i1 = SIZE_MAX;
    float best_dist = INFINITY;
    float second_best_dist = INFINITY;
    for (size_t i1 = 0; i1 < descriptors1.shape(0); ++i1) {
        float dist = 0;
        for (size_t d = 0; d < descriptors1.shape(1); ++d) {
            dist += squared(descriptor0[d] - descriptors1(i1, d));
        }
        if (dist < best_dist) {
            best_i1 = i1;
            second_best_dist = best_dist;
            best_dist = dist;
        }
    }
    if (best_dist < squared(lowe_ratio) * second_best_dist) {
        return best_i1;
    } else {
        return SIZE_MAX;
    }
}
