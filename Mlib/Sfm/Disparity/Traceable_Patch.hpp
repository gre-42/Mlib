#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib::Sfm {

class TraceablePatch {
public:
    TraceablePatch(
        const Array<float>& image,
        const FixedArray<size_t, 2>& patch_center,
        const FixedArray<size_t, 2>& patch_size,
        const FixedArray<size_t, 2>& patch_nan_size = FixedArray<size_t, 2>{0u, 0u},
        size_t min_npixels = 0,  // 0 = enable strict mode);
        float min_brightness = 0.1f);
    FixedArray<size_t, 2> new_position_in_box(
        const Array<float>& image,
        const FixedArray<size_t, 2>& patch_center,
        const FixedArray<size_t, 2>& search_window,
        float worst_error) const;
    float new_position_on_line(
        const Array<float>& image,
        const FixedArray<size_t, 2>& patch_center,
        const FixedArray<float, 2>& direction,
        size_t search_length,
        float worst_error,
        float* out_error = nullptr,
        float* prior_disparity = nullptr,
        float* prior_strength = nullptr) const;
    FixedArray<float, 2> new_position_in_candidate_list(
        const Array<float>& image,
        const FixedArray<float, 2>& patch_center,
        const Array<FixedArray<float, 2>>& candidates,
        float worst_error,
        float lowe_ratio = 0.75f) const;
    float error_at_position(
        const Array<float>& image,
        const FixedArray<size_t, 2>& patch_center) const;
    Array<float> image_patch_;
    bool good_;
private:
    float brightness_;
    size_t min_npixels_;
    float min_brightness_;
};

}
