#include "Traceable_Patch.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>

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
    const FixedArray<size_t, 2>& patch_center,
    const FixedArray<size_t, 2>& patch_size,
    const FixedArray<size_t, 2>& patch_nan_size,
    size_t min_npixels,
    float min_brightness)
: image_patch_{ ArrayShape{ image.shape(0), patch_size(0), patch_size(1) } },
  brightness_{0},
  min_npixels_{min_npixels},
  min_brightness_{min_brightness}
{
    assert(image.ndim() == 3); // channel, rows, columns
    FixedArray<size_t, 2> image_shape2{ image.shape(1), image.shape(2) };
    FixedArray<size_t, 2> image_patch_shape2{ image_patch_.shape(1), image_patch_.shape(2) };
    size_t npixels = 0;
    for (size_t r = 0; r < image_patch_.shape(1); ++r) {
        for (size_t c = 0; c < image_patch_.shape(2); ++c) {
            FixedArray<size_t, 2> rc{r, c};
            FixedArray<size_t, 2> index =
                patch_center
                + rc
                - image_patch_shape2 / (size_t)2;
            if (all(index < image_shape2) &&
                (any(rc >= image_patch_shape2 / (size_t)2 + patch_nan_size) ||
                 any(rc + patch_nan_size <= image_patch_shape2 / (size_t)2)) &&
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
    good_ = (npixels >= (min_npixels != 0 ? min_npixels : prod(image_patch_shape2)));
    if (good_) {
        brightness_ /= (npixels * image.shape(0));
    }
    // lerr() << image_patch_;
}

float TraceablePatch::error_at_position(
    const Array<float>& image,
    const FixedArray<size_t, 2>& patch_center) const
{
    assert_true(good_);
    assert(image.ndim() == 3); // channel, rows, columns
    assert(image.shape(0) == 3 || image.shape(0) == 6);
    float error_sum = 0;
    size_t nerror = 0;
    FixedArray<size_t, 2> image_shape2{ image.shape(1), image.shape(2) };
    FixedArray<size_t, 2> image_patch_shape2{ image_patch_.shape(1), image_patch_.shape(2) };
    for (size_t r = 0; r < image_patch_.shape(1); ++r) {
        for (size_t c = 0; c < image_patch_.shape(2); ++c) {
            size_t fixed_index_0 = patch_center(0) + r - image_patch_shape2(0) / 2;
            size_t fixed_index_1 = patch_center(1) + c - image_patch_shape2(1) / 2;
            size_t moving_index_0 = r;
            size_t moving_index_1 = c;
            // lerr() << fixed_index << " - " << moving_index << " - " << image_shape2;
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
    if ((brightness_ > min_brightness_) && nerror >= (min_npixels_ != 0 ? min_npixels_ : prod(image_patch_shape2))) {
        return error_sum / float(image.shape(0)) / nerror / brightness_;
    } else {
        return std::numeric_limits<float>::infinity();
    }
}

FixedArray<size_t, 2> TraceablePatch::new_position_in_box(
    const Array<float>& image,
    const FixedArray<size_t, 2>& patch_center,
    const FixedArray<size_t, 2>& search_window,
    float worst_error) const
{
    assert_true(good_);
    FixedArray<size_t, 2> image_shape2{ image.shape(1), image.shape(2) };
    float best_error = std::numeric_limits<float>::infinity();
    FixedArray<size_t, 2> best_id{SIZE_MAX, SIZE_MAX};
    size_t ncandidates = 0;
    for (size_t dr = 0; dr < search_window(0); ++dr) {
        for (size_t dc = 0; dc < search_window(1); ++dc) {
            FixedArray<size_t, 2> id = patch_center + FixedArray<size_t, 2>{dr, dc} - search_window / (size_t)2;
            // Negative indices are not (yet) supported on the caller-site.
            // The image is in RGB, so remove the first dimension.
            if (all(id < image_shape2)) {
                float error = error_at_position(image, id);
                // lerr()  << dr << " - " << dc << " | " << error << " - " << best_error;
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
    if ((min_npixels_ == 0) && (ncandidates != prod(search_window))) {
        return FixedArray<size_t, 2>{SIZE_MAX, SIZE_MAX};
    }
    // lerr() << "best " << best_id;
    // lerr() << "best error " << best_error;
    return best_id;
}

float TraceablePatch::new_position_on_line(
    const Array<float>& image,
    const FixedArray<size_t, 2>& patch_center,
    const FixedArray<float, 2>& direction,
    size_t search_length,
    float worst_error,
    float* out_error,
    float* prior_disparity,
    float* prior_strength) const
{
    FixedArray<size_t, 2> image_shape2{ image.shape(1), image.shape(2) };
    float best_error = INFINITY;
    float best_disparity = NAN;
    for (float s = -float(search_length); s <= float(search_length); ++s) {
        FixedArray<size_t, 2> id = patch_center + a2i(s * direction);
        // Negative indices are not (yet) supported on the caller-site.
        // The image is in RGB, so remove the first dimension.
        if (all(id < image_shape2)) {
            float error = error_at_position(image, id);
            if (prior_disparity != nullptr && prior_strength != nullptr) {
                // lerr() << "err " << error << " + " << squared(s - *prior_disparity) * (*prior_strength);
                error += squared(s - *prior_disparity) * (*prior_strength);
            }
            // lerr()  << id << " | " << error << " - " << best_error;
            if ((error < best_error) && (error <= worst_error)) {
                best_error = error;
                best_disparity = s;
            }
        }
    }
    if (out_error != nullptr) {
        *out_error = best_error;
    }
    // lerr() << "best " << best_id;
    // lerr() << "best error " << best_error;
    return best_disparity;
}

FixedArray<float, 2> TraceablePatch::new_position_in_candidate_list(
    const Array<float>& image,
    const FixedArray<float, 2>& patch_center,
    const Array<FixedArray<float, 2>>& candidates,
    float worst_error,
    float lowe_ratio) const
{
    assert_true(good_);
    FixedArray<size_t, 2> image_shape2{ image.shape(1), image.shape(2) };
    float second_best_error = std::numeric_limits<float>::infinity();
    float best_error = std::numeric_limits<float>::infinity();
    FixedArray<size_t, 2> best_id{SIZE_MAX, SIZE_MAX};
    for (const FixedArray<float, 2>& patch_center : candidates.flat_iterable()) {
        FixedArray<size_t, 2> id = a2i(patch_center);
        // Negative indices are not (yet) supported on the caller-site.
        // The image is in RGB, so remove the first dimension.
        if (all(id < image_shape2)) {
            float error = error_at_position(image, id);
            // lerr()  << dr << " - " << dc << " | " << error << " - " << best_error;
            if ((error < best_error) && (error <= worst_error)) {
                second_best_error = best_error;
                best_error = error;
                best_id = id;
            }
        }
    }
    if (best_error > lowe_ratio * second_best_error) {
        best_id = SIZE_MAX;
    }
    // lerr() << "best " << best_id;
    // lerr() << "best error " << best_error;
    return i2a(best_id);
}
