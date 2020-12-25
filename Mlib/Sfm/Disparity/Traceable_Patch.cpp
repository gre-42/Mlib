#include "Traceable_Patch.hpp"
#include <Mlib/Images/Coordinates.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

bool any_channel_nan(const Array<float>& image, size_t r, size_t c) {
    for (size_t h = 0; h < image.shape(0); ++h) {
        if (std::isnan(image(h, r, c))) {
            return true;
        }
    }
    return false;
}

TraceablePatch::TraceablePatch(
    const Array<float>& image,
    const ArrayShape& patch_center,
    const ArrayShape& patch_size,
    const ArrayShape& patch_nan_size,
    size_t min_npixels)
: image_patch_{ArrayShape{image.shape(0)}.concatenated(patch_size)},
  brightness_{0},
  min_npixels_{min_npixels}
{
    assert(image.ndim() == 3); // channel, rows, columns
    ArrayShape image_shape2 = image.shape().erased_first();
    ArrayShape image_patch_shape2 = image_patch_.shape().erased_first();
    size_t npixels = 0;
    for (size_t r = 0; r < image_patch_.shape(1); ++r) {
        for (size_t c = 0; c < image_patch_.shape(2); ++c) {
            ArrayShape rc{r, c};
            ArrayShape index =
                patch_center
                + rc
                - image_patch_shape2 / 2;
            if (all(index < image_shape2) &&
                (any(rc >= image_patch_shape2 / 2 + patch_nan_size) ||
                 any(rc + patch_nan_size <= image_patch_shape2 / 2)) &&
                !any_channel_nan(image, index(0), index(1))) {
                for (size_t h = 0; h < image_patch_.shape(0); ++h) {
                    brightness_ += (image_patch_(h, r, c) = image[h](index));
                }
                ++npixels;
            } else {
                for (size_t h = 0; h < image_patch_.shape(0); ++h) {
                    image_patch_(h, r, c) = NAN;
                }
            }
        }
    }
    good_ = (npixels >= (min_npixels != 0 ? min_npixels : image_patch_shape2.nelements()));
    if (good_) {
        brightness_ /= (npixels * image.shape(0));
    }
    // std::cerr << image_patch_ << std::endl;
}

float TraceablePatch::error_at_position(
    const Array<float>& image,
    const ArrayShape& patch_center) const
{
    assert_true(good_);
    assert(image.ndim() == 3); // channel, rows, columns
    assert(image.shape(0) == 3 || image.shape(0) == 6);
    float error_sum = 0;
    size_t nerror = 0;
    ArrayShape image_shape2 = image.shape().erased_first();
    ArrayShape image_patch_shape2 = image_patch_.shape().erased_first();
    for (size_t r = 0; r < image_patch_.shape(1); ++r) {
        for (size_t c = 0; c < image_patch_.shape(2); ++c) {
            size_t fixed_index_0 = patch_center(0) + r - image_patch_shape2(0) / 2;
            size_t fixed_index_1 = patch_center(1) + c - image_patch_shape2(1) / 2;
            size_t moving_index_0 = r;
            size_t moving_index_1 = c;
            // std::cerr << fixed_index << " - " << moving_index << " - " << image_shape2 << std::endl;
            if (fixed_index_0 < image_shape2(0) &&
                fixed_index_1 < image_shape2(1) &&
                !any_channel_nan(image_patch_, moving_index_0, moving_index_1) &&
                !any_channel_nan(image, fixed_index_0, fixed_index_1))
            {
                for (size_t h = 0; h < image.shape(0); ++h) {
                    error_sum += std::abs(image_patch_(h, moving_index_0, moving_index_1) - image(h, fixed_index_0, fixed_index_1));
                }
                ++nerror;
            }
        }
    }
    if (nerror >= (min_npixels_ != 0 ? min_npixels_ : image_patch_shape2.nelements())) {
        return error_sum / float(image.shape(0)) / nerror / brightness_;
    } else {
        return std::numeric_limits<float>::infinity();
    }
}

ArrayShape TraceablePatch::new_position_in_box(
    const Array<float>& image,
    const ArrayShape& patch_center,
    const ArrayShape& search_window,
    float worst_error) const
{
    assert_true(good_);
    float best_error = std::numeric_limits<float>::infinity();
    ArrayShape best_id{SIZE_MAX, SIZE_MAX};
    size_t ncandidates = 0;
    for (size_t dr = 0; dr < search_window(0); ++dr) {
        for (size_t dc = 0; dc < search_window(1); ++dc) {
            ArrayShape id = patch_center + ArrayShape{dr, dc} - search_window / 2;
            // Negative indices are not (yet) supported on the caller-site.
            // The image is in RGB, so remove the first dimension.
            if (all(id < image.shape().erased_first())) {
                float error = error_at_position(image, id);
                // std::cerr  << dr << " - " << dc << " | " << error << " - " << best_error << std::endl;
                if ((error < best_error) && (error <= worst_error)) {
                    best_error = error;
                    best_id = id;
                }
                if (error != INFINITY) {
                    ++ncandidates;
                }
            }
        }
    }
    if ((min_npixels_ == 0) && (ncandidates != search_window.nelements())) {
        return ArrayShape{SIZE_MAX, SIZE_MAX};
    }
    // std::cerr << "best " << best_id << std::endl;
    // std::cerr << "best error " << best_error << std::endl;
    return best_id;
}

float TraceablePatch::new_position_on_line(
    const Array<float>& image,
    const ArrayShape& patch_center,
    const Array<float>& direction,
    size_t search_length,
    float worst_error,
    float* out_error,
    float* prior_disparity,
    float* prior_strength) const
{
    float best_error = INFINITY;
    float best_disparity = NAN;
    for (float s = -float(search_length); s <= float(search_length); ++s) {
        ArrayShape id = patch_center + a2i(s * direction);
        // Negative indices are not (yet) supported on the caller-site.
        // The image is in RGB, so remove the first dimension.
        if (all(id < image.shape().erased_first())) {
            float error = error_at_position(image, id);
            if (prior_disparity != nullptr && prior_strength != nullptr) {
                // std::cerr << "err " << error << " + " << squared(s - *prior_disparity) * (*prior_strength) << std::endl;
                error += squared(s - *prior_disparity) * (*prior_strength);
            }
            // std::cerr  << id << " | " << error << " - " << best_error << std::endl;
            if ((error < best_error) && (error <= worst_error)) {
                best_error = error;
                best_disparity = s;
            }
        }
    }
    if (out_error != nullptr) {
        *out_error = best_error;
    }
    // std::cerr << "best " << best_id << std::endl;
    // std::cerr << "best error " << best_error << std::endl;
    return best_disparity;
}
