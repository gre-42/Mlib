#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib::Sfm {

class TraceableDescriptor {
public:
    explicit TraceableDescriptor(const Array<float>& descriptor);
    bool new_position_in_candidate_list(
        FixedArray<float, 2>& position,
        const Array<float>& feature_points1,
        const Array<float>& descriptors1,
        float lowe_ratio = 0.75f) const;
    size_t descriptor_id_in_parameter_list(
        const Array<float>& descriptors1,
        float lowe_ratio = 0.75f);
    static size_t descriptor_id_in_parameter_list(
        const float* descriptor0,
        const Array<float>& descriptors1,
        float lowe_ratio = 0.75f);
private:
    Array<float> descriptor_;
};

}
