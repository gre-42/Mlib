#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Array_Shape.hpp>

namespace Mlib { namespace Sfm {

Array<float> harris_response_1d(
    const Array<float>& im1,
    const Array<float>& F);

/**
 * Cf. "CorrespondingFeaturesOnLine"
 */
Array<float> compute_disparity_gray_single_pixel(
    const Array<float>& im0,
    const Array<float>& im1,
    const Array<float>& F,
    size_t search_length,
    bool l1_normalization = true,
    Array<float>* dsi = nullptr,
    float d_multiplier = 1);

Array<float> compute_disparity_rgb_single_pixel(
    const Array<float>& im0,
    const Array<float>& im1,
    const Array<float>& F,
    size_t search_length,
    bool l1_normalization = true,
    Array<float>* dsi = nullptr,
    float d_multiplier = 1);

Array<float> compute_disparity_rgb_patch(
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const Array<float>& F,
    size_t search_length,
    float worst_error,
    const ArrayShape& patch_size = ArrayShape{10, 10},
    const ArrayShape& patch_nan_size = ArrayShape{0, 0},
    Array<float>* im0_error = nullptr,
    Array<bool>* mask = nullptr,
    Array<float>* prior_disparity = nullptr,
    Array<float>* prior_strength = nullptr);

Array<float> iterate_disparity_rgb_patch(
    const Array<float>& im0_gray,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const Array<float>& F,
    size_t search_length,
    float worst_error,
    const ArrayShape& patch_size = ArrayShape{10, 10},
    const ArrayShape& patch_nan_size = ArrayShape{0, 0},
    Array<float>* im0_error = nullptr,
    Array<float>* initial_disparity = nullptr,
    bool draw_debug_images = false);

Array<float> move_along_disparity(
    const Array<float>& disparity,
    const Array<float>& F,
    const Array<float>& img1_rgb,
    const Array<bool>* mask = nullptr,
    Array<bool>* reverse_mask = nullptr);

Array<float> reconstruct_disparity(
    const Array<float>& disparity,
    const Array<float>& F,
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& intrinsic_matrix,
    Array<float>* condition_number = nullptr);

Array<float> reconstruct_depth(
    const Array<float>& depth,
    const Array<float>& intrinsic_matrix);

}}
