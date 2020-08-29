#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class TraceablePatch {
public:
    TraceablePatch(
        const Array<float>& image,
        const ArrayShape& patch_center,
        const ArrayShape& patch_size,
        const ArrayShape& patch_nan_size = ArrayShape{0, 0},
        size_t min_npixels = 0);  // 0 = enable strict mode);
    ArrayShape new_position_in_box(
        const Array<float>& image,
        const ArrayShape& patch_center,
        const ArrayShape& search_window,
        float worst_error) const;
    float new_position_on_line(
        const Array<float>& image,
        const ArrayShape& patch_center,
        const Array<float>& direction,
        size_t search_length,
        float worst_error,
        float* out_error = nullptr,
        float* prior_disparity = nullptr,
        float* prior_strength = nullptr) const;
    float error_at_position(
        const Array<float>& image,
        const ArrayShape& patch_center) const;
    Array<float> image_patch_;
    bool good_;
private:
    float brightness_;
    size_t min_npixels_;
};

}}
