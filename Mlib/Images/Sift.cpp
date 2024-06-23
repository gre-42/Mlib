#include "Sift.hpp"
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Resample/Pyramid.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Fixed_Trace.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <set>

using namespace Mlib;
using namespace Mlib::Sift;

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif
static const float float_tolerance = float(1e-7);

Array<float> subtract(const Array<float>& a, const Array<float>& b) {
    return a - b;
}

// static const float fac = 255.f;
static const float fac = 1.f;

template <class T>
inline T rad2deg(T rad) {
    return (T)180/(T)M_PI * rad;
}

template <class T>
inline T deg2rad(T deg) {
    return (T)M_PI / (T)180 * deg;
}

// #########################
// # Image pyramid related #
// #########################

Array<float> generateBaseImage(const Array<float>& image, float sigma, float assumed_blur)
{
    // Generate base image from input image by upsampling by 2 in both directions and blurring
    //
    // logger.debug('Generating base image...')
    Array<float> result = up_sample2(image);
    float sigma_diff = std::sqrt(std::max(squared(sigma) - squared((2 * assumed_blur)), 0.01f));
    return gaussian_filter_NWE(image, sigma_diff, NAN);  // the image blur is now sigma instead of assumed_blur
}

size_t computeNumberOfOctaves(size_t nrows, size_t ncols) {
    // Compute number of octaves in image pyramid as function of base image shape (OpenCV default)
    //
    return size_t(std::round(std::log(std::min(nrows, ncols)) / std::log(2) - 1));
}

Array<float> generateGaussianKernels(float sigma, size_t num_intervals) {
    // Generate list of gaussian kernels at which to blur the input image. Default values of sigma, intervals, and octaves follow section 3 of Lowe's paper.
    //
    // logger.debug('Generating scales...')
    size_t num_images_per_octave = num_intervals + 3;
    float k = std::pow(2.f, (1.f / (float)num_intervals));
    Array<float> gaussian_kernels = zeros<float>(ArrayShape{ num_images_per_octave} );  // scale of gaussian blur necessary to go from one blur scale to the next within an octave
    gaussian_kernels[0] = sigma;

    for (size_t image_index = 1; image_index < num_images_per_octave; ++image_index) {
        float sigma_previous = std::pow(k, float(image_index - 1)) * sigma;
        float sigma_total = k * sigma_previous;
        gaussian_kernels(image_index) = std::sqrt(squared(sigma_total) - squared(sigma_previous));
    }
    return gaussian_kernels;
}

std::vector<std::vector<Array<float>>> generateGaussianImages(const Array<float>& image, size_t num_octaves, const Array<float>& gaussian_kernels) {
    // Generate scale-space pyramid of Gaussian images
    //
    // logger.debug('Generating Gaussian images...')
    std::vector<std::vector<Array<float>>> gaussian_images;
    gaussian_images.reserve(num_octaves);

    Array<float> img = image;

    for (size_t octave_index = 0; octave_index < num_octaves; ++octave_index) {
        std::vector<Array<float>> gaussian_images_in_octave;
        gaussian_images_in_octave.reserve(gaussian_kernels.length());
        gaussian_images_in_octave.push_back(img);  // first image in octave already has the correct blur
        for (size_t i = 1; i < gaussian_kernels.length(); ++i) {
            img.move() = gaussian_filter_NWE(img,  gaussian_kernels(i), NAN);
            gaussian_images_in_octave.push_back(img);
        }
        gaussian_images.push_back(gaussian_images_in_octave);
        Array<float> octave_base = gaussian_images_in_octave[gaussian_images_in_octave.size() - 3];
        img.move() = down_sample2(img);
    }
    return gaussian_images;
}

std::vector<std::vector<Array<float>>> generateDoGImages(const std::vector<std::vector<Array<float>>>& gaussian_images) {
    // Generate Difference-of-Gaussians image pyramid
    //
    // logger.debug('Generating Difference-of-Gaussian images...')
    std::vector<std::vector<Array<float>>> dog_images;
    dog_images.reserve(gaussian_images.size());

    for (const auto& gaussian_images_in_octave : gaussian_images) {
        std::vector<Array<float>> dog_images_in_octave;
        dog_images_in_octave.reserve(gaussian_images_in_octave.size() - 1);
        for (size_t first_image = 0; first_image < gaussian_images_in_octave.size() - 1; ++first_image) {
            dog_images_in_octave.push_back(subtract(gaussian_images_in_octave[first_image + 1], gaussian_images_in_octave[first_image]));  // ordinary subtraction will not work because the images are unsigned integers
        }
        dog_images.push_back(dog_images_in_octave);
    }
    return dog_images;
}

// ###############################
// # Scale-space extrema related #
// ###############################

bool isPixelAnExtremum(
    size_t i,
    size_t j,
    const Array<float>& first_subimage,
    const Array<float>& second_subimage,
    const Array<float>& third_subimage,
    float threshold)
{
    // [i-1:i+2, j-1:j+2]
    // Return True if the center element of the 3x3x3 input array is strictly greater than or less than all its neighbors, False otherwise
    //
    float center_pixel_value = second_subimage(i, j);
    if (std::abs(center_pixel_value) > threshold) {
        if (center_pixel_value > 0) {
            for (size_t r = std::max(i, (size_t)1) - 1; r < std::min(first_subimage.shape(0), i + 1); ++r) {
                for (size_t c = std::max(j, (size_t)1) - 1; c < std::min(first_subimage.shape(1), j + 1); ++c) {
                    if (r == i && c == j) {
                        continue;
                    }
                    if (center_pixel_value < first_subimage(r, c) ) {
                        return false;
                    }
                    if (center_pixel_value < second_subimage(r, c) ) {
                        return false;
                    }
                    if (center_pixel_value < third_subimage(r, c) ) {
                        return false;
                    }
                }
            }
            return true;
        } else if (center_pixel_value < 0) {
            for (size_t r = std::max(i, (size_t)1) - 1; r < std::min(first_subimage.shape(0), i + 1); ++r) {
                for (size_t c = std::max(j, (size_t)1) - 1; c < std::min(first_subimage.shape(1), j + 1); ++c) {
                    if (r == i && c == j) {
                        continue;
                    }
                    if (center_pixel_value > first_subimage(r, c) ) {
                        return false;
                    }
                    if (center_pixel_value > second_subimage(r, c) ) {
                        return false;
                    }
                    if (center_pixel_value > third_subimage(r, c) ) {
                        return false;
                    }
                }
            }
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

FixedArray<float, 3> computeGradientAtCenterPixel(const FixedArray<float, 3, 3, 3>& pixel_array)
{
    // Approximate gradient at center pixel [1, 1, 1] of 3x3x3 array using central difference formula of order O(h^2), where h is the step size
    //
    // With step size h, the central difference formula of order O(h^2) for f'(x) is (f(x + h) - f(x - h)) / (2 * h)
    // Here h = 1, so the formula simplifies to f'(x) = (f(x + 1) - f(x - 1)) / 2
    // NOTE: x corresponds to second array axis, y corresponds to first array axis, and s (scale) corresponds to third array axis
    return FixedArray<float, 3>{
        0.5f * (pixel_array(1, 1, 2) - pixel_array(1, 1, 0)),
        0.5f * (pixel_array(1, 2, 1) - pixel_array(1, 0, 1)),
        0.5f * (pixel_array(2, 1, 1) - pixel_array(0, 1, 1))};
}

FixedArray<float, 3, 3> computeHessianAtCenterPixel(const FixedArray<float, 3, 3, 3>& pixel_array)
{
    // Approximate Hessian at center pixel [1, 1, 1] of 3x3x3 array using central difference formula of order O(h^2), where h is the step size
    //
    // With step size h, the central difference formula of order O(h^2) for f''(x) is (f(x + h) - 2 * f(x) + f(x - h)) / (h ^ 2)
    // Here h = 1, so the formula simplifies to f''(x) = f(x + 1) - 2 * f(x) + f(x - 1)
    // With step size h, the central difference formula of order O(h^2) for (d^2) f(x, y) / (dx dy) = (f(x + h, y + h) - f(x + h, y - h) - f(x - h, y + h) + f(x - h, y - h)) / (4 * h ^ 2)
    // Here h = 1, so the formula simplifies to (d^2) f(x, y) / (dx dy) = (f(x + 1, y + 1) - f(x + 1, y - 1) - f(x - 1, y + 1) + f(x - 1, y - 1)) / 4
    // NOTE: x corresponds to second array axis, y corresponds to first array axis, and s (scale) corresponds to third array axis
    float center_pixel_value = pixel_array(1, 1, 1);
    float dxx = pixel_array(1, 1, 2) - 2 * center_pixel_value + pixel_array(1, 1, 0);
    float dyy = pixel_array(1, 2, 1) - 2 * center_pixel_value + pixel_array(1, 0, 1);
    float dss = pixel_array(2, 1, 1) - 2 * center_pixel_value + pixel_array(0, 1, 1);
    float dxy = 0.25f * (pixel_array(1, 2, 2) - pixel_array(1, 2, 0) - pixel_array(1, 0, 2) + pixel_array(1, 0, 0));
    float dxs = 0.25f * (pixel_array(2, 1, 2) - pixel_array(2, 1, 0) - pixel_array(0, 1, 2) + pixel_array(0, 1, 0));
    float dys = 0.25f * (pixel_array(2, 2, 1) - pixel_array(2, 0, 1) - pixel_array(0, 2, 1) + pixel_array(0, 0, 1));
    return FixedArray<float, 3, 3>::init(
        dxx, dxy, dxs,
        dxy, dyy, dys,
        dxs, dys, dss);
}

bool localizeExtremumViaQuadraticFit(
    size_t i,
    size_t j,
    size_t& image_index,
    KeyPoint& keypoint,
    size_t octave_index,
    size_t num_intervals,
    const std::vector<Array<float>>& dog_images_in_octave,
    float sigma,
    float contrast_threshold,
    size_t image_border_width,
    float eigenvalue_ratio=10,
    size_t num_attempts_until_convergence=5)
{
    if (num_attempts_until_convergence == 0) {
        THROW_OR_ABORT("num_attempts_until_convergence must be > 0");
    }
    // Iteratively refine pixel positions of scale-space extrema via quadratic fit around each extremum's neighbors
    //
    // logger.debug('Localizing scale-space extrema...')
    bool extremum_is_outside_image = false;
    FixedArray<size_t, 2> image_shape = dog_images_in_octave[0].fixed_shape<2>();
    size_t attempt_index;
    FixedArray<float, 3, 3, 3> pixel_cube = uninitialized;
    FixedArray<float, 3> gradient = uninitialized;
    FixedArray<float, 3, 3> hessian = uninitialized;
    FixedArray<float, 3> extremum_update = uninitialized;
    for (attempt_index = 0; attempt_index < num_attempts_until_convergence; ++attempt_index) {
        // need to convert from uint8 to float32 to compute derivatives and need to rescale pixel values to [0, 1] to apply Lowe's thresholds
        for (size_t h = 0; h < 3; ++h) {
            for (size_t r = 0; r < 3; ++r) {
                for (size_t c = 0; c < 3; ++c) {
                    pixel_cube(h, r, c) = dog_images_in_octave[image_index + h - 1](i + r - 1, j + c - 1) / fac;
                }
            }
        }
        gradient = computeGradientAtCenterPixel(pixel_cube);
        hessian = computeHessianAtCenterPixel(pixel_cube);
        extremum_update = -lstsq_chol_1d(hessian, gradient).value();
        if (std::abs(extremum_update(0)) < 0.5f && std::abs(extremum_update(1)) < 0.5f && std::abs(extremum_update(2)) < 0.5f)
        {
            break;
        }
        j += (size_t)int(round(extremum_update(0)));
        i += (size_t)int(round(extremum_update(1)));
        image_index += (size_t)int(round(extremum_update(2)));
        // make sure the new pixel_cube will lie entirely within the image
        if (i < image_border_width ||
            i + image_border_width >= image_shape(0) ||
            j < image_border_width ||
            j + image_border_width >= image_shape(1) ||
            image_index < 1 ||
            image_index > num_intervals)
        {
            extremum_is_outside_image = true;
            break;
        }
    }
    if (extremum_is_outside_image) {
        // logger.debug('Updated extremum moved outside of image before reaching convergence. Skipping...')
        return false;
    }
    if (attempt_index >= num_attempts_until_convergence - 1) {
        // logger.debug('Exceeded maximum number of attempts without reaching convergence for this extremum. Skipping...')
        return false;
    }
    float functionValueAtUpdatedExtremum = pixel_cube(1, 1, 1) + 0.5f * dot0d(gradient, extremum_update);
    if (std::abs(functionValueAtUpdatedExtremum) * float(num_intervals) >= contrast_threshold) {
        auto xy_hessian = FixedArray<float, 2, 2>::init(
            hessian(0, 0), hessian(0, 1),
            hessian(1, 0), hessian(1, 1));
        float xy_hessian_trace = trace2x2(xy_hessian);
        float xy_hessian_det = det2x2(xy_hessian);
        if (xy_hessian_det > 0 and eigenvalue_ratio * squared(xy_hessian_trace) < squared((eigenvalue_ratio + 1)) * xy_hessian_det) {
            // Contrast check passed -- construct and return OpenCV KeyPoint object
            keypoint.pt = FixedArray<float, 2>{
                ((float)j + extremum_update(0)) * std::pow(2.f, (float)octave_index),
                ((float)i + extremum_update(1)) * std::pow(2.f, (float)octave_index)};
            keypoint.octave = (int)octave_index + (int)image_index * (int)std::pow(2, 8) + int(round((extremum_update(2) + 0.5) * 255.)) * (int)std::pow(2, 16);
            keypoint.size = sigma * std::pow(2.f, (((float)image_index + extremum_update(2)) / (float)num_intervals)) * std::pow(2.f, float(octave_index + 1));  // octave_index + 1 because the input image was doubled
            keypoint.response = std::abs(functionValueAtUpdatedExtremum);
            return true;
        }
    }
    return false;
}

// #########################
// # Keypoint orientations #
// #########################

std::list<KeyPointWithOrientation> computeKeypointsWithOrientations(
    const KeyPoint& keypoint,
    size_t octave_index,
    const Array<float>& gaussian_image,
    size_t radius_factor=3,
    size_t num_bins=36,
    float peak_ratio=0.8f,
    float scale_factor=1.5f)
{
    // Compute orientations for each keypoint
    //
    // logger.debug('Computing keypoint orientations...')
    std::list<KeyPointWithOrientation> keypoints_with_orientations;
    FixedArray<size_t, 2> image_shape = gaussian_image.fixed_shape<2>();

    float scale = scale_factor * keypoint.size / (float)std::pow(2, (octave_index + 1));  // compare with keypoint.size computation in localizeExtremumViaQuadraticFit()
    int radius = int(round((float)radius_factor * scale));
    float weight_factor = -0.5f / squared(scale);
    Array<float> raw_histogram = zeros<float>(ArrayShape{ num_bins });
    Array<float> smooth_histogram = zeros<float>(ArrayShape{ num_bins });

    for (int i = -radius; i < radius + 1; ++i) {
        int region_y = int(round(keypoint.pt(1) / (float)std::pow(2, octave_index))) + i;
        if ((region_y > 0) && (region_y < (int)image_shape(0) - 1)) {
            for (int j = -radius; j < radius + 1; ++j) {
                int region_x = int(round(keypoint.pt(0) / (float)std::pow(2, octave_index))) + j;
                if (region_x > 0 && region_x < (int)image_shape(1) - 1) {
                    float dx = gaussian_image((size_t)region_y, (size_t)region_x + 1) - gaussian_image((size_t)region_y, (size_t)region_x - 1);
                    float dy = gaussian_image((size_t)region_y - 1, (size_t)region_x) - gaussian_image((size_t)region_y + 1, (size_t)region_x);
                    float gradient_magnitude = std::sqrt(dx * dx + dy * dy);
                    float gradient_orientation = rad2deg(std::atan2(dy, dx));
                    float weight = std::exp(weight_factor * float(squared(i) + squared(j)));  // constant in front of exponential can be dropped because we will find peaks later
                    size_t histogram_index = size_t(round(gradient_orientation * float(num_bins) / 360.f));
                    raw_histogram(histogram_index % num_bins) += weight * gradient_magnitude;
                }
            }
        }
    }

    for (size_t n = 0; n < num_bins; ++n) {
        smooth_histogram(n) =
            (6.f * raw_histogram((size_t)n) +
             4.f * (raw_histogram((size_t)n - 1) + raw_histogram(((size_t)n + 1) % num_bins)) +
             raw_histogram((size_t)n - 2) +
             raw_histogram(((size_t)n + 2) % num_bins)) / 16.f;
    }
    float orientation_max = max(smooth_histogram);
    Array<size_t> orientation_peaks{ ArrayShape{0} };
    auto calculate_orientation_peak = [&smooth_histogram, &orientation_peaks](size_t i, size_t l, size_t r) {
        if ((smooth_histogram(i) > smooth_histogram(l)) && (smooth_histogram(i) > smooth_histogram(r))) {
            orientation_peaks.append(i);
        }
    };
    calculate_orientation_peak(0, orientation_peaks.length() - 1, 1);
    calculate_orientation_peak(orientation_peaks.length() - 1, orientation_peaks.length() - 2, 0);
    for (size_t i = 1; i < smooth_histogram.length() - 1; ++i) {
        calculate_orientation_peak((size_t)i, (size_t)i - 1, (size_t)i + 1);
    }
    for (size_t peak_index : orientation_peaks.flat_iterable())
    {
        float peak_value = smooth_histogram(peak_index);
        if (peak_value >= peak_ratio * orientation_max) {
            if (peak_index == 0) {
                THROW_OR_ABORT("Peak index is zero");
            }
            // Quadratic peak interpolation
            // The interpolation update is given by equation (6.30) in https://ccrma.stanford.edu/~jos/sasp/Quadratic_Interpolation_Spectral_Peaks.html
            float left_value = smooth_histogram((peak_index - 1) % num_bins);
            float right_value = smooth_histogram((peak_index + 1) % num_bins);
            float interpolated_peak_index = std::fmod((float)peak_index + 0.5f * (left_value - right_value) / (left_value - 2.f * peak_value + right_value), (float)num_bins);
            float angle = 360.f - interpolated_peak_index * 360.f / (float)num_bins;
            if (std::abs(angle - 360.f) < float_tolerance) {
                angle = 0.f;
            }
            keypoints_with_orientations.push_back(KeyPointWithOrientation{
                .kp = keypoint,
                .angle = angle});
        }
    }
    return keypoints_with_orientations;
}

std::list<KeyPointWithOrientation> findScaleSpaceExtrema(
    const std::vector<std::vector<Array<float>>>& gaussian_images,
    const std::vector<std::vector<Array<float>>>& dog_images,
    size_t num_intervals,
    float sigma,
    size_t image_border_width,
    float contrast_threshold=0.04f)
{
    // Find pixel positions of all scale-space extrema in the image pyramid
    //
    // logger.debug('Finding scale-space extrema...')
    float threshold = std::floor(0.5f * contrast_threshold / (float)num_intervals * 255.f);  // from OpenCV implementation
    std::list<KeyPointWithOrientation> keypoints;

    for (size_t octave_index = 0; octave_index < dog_images.size(); ++octave_index) {
        const std::vector<Array<float>>& dog_images_in_octave = dog_images[octave_index];
        for (size_t image_index = 0; image_index < dog_images_in_octave.size() - 2; ++image_index) {
            const auto& first_image = dog_images_in_octave[image_index];
            const auto& second_image = dog_images_in_octave[image_index + 1];
            const auto& third_image = dog_images_in_octave[image_index + 2];
            // (i, j) is the center of the 3x3 array
            for (size_t i = image_border_width; i + image_border_width < first_image.shape(0); ++i) {
                for (size_t j = image_border_width; j + image_border_width < first_image.shape(1); ++j) {
                    if (isPixelAnExtremum(i, j, first_image, second_image, third_image, threshold)) {
                        KeyPoint keypoint;
                        size_t localized_image_index = image_index + 1;
                        if (localizeExtremumViaQuadraticFit(i, j, localized_image_index, keypoint, octave_index, num_intervals, dog_images_in_octave, sigma, contrast_threshold, image_border_width))
                        {
                            std::list<KeyPointWithOrientation> keypoints_with_orientations = computeKeypointsWithOrientations(keypoint, octave_index, gaussian_images[octave_index][localized_image_index]);
                            for (const auto& keypoint_with_orientation : keypoints_with_orientations) {
                                keypoints.push_back(keypoint_with_orientation);
                            }
                        }
                    }
                }
            }
        }
    }
    return keypoints;
}

// ##############################
// # Duplicate keypoint removal #
// ##############################

void removeDuplicateKeypoints(std::list<KeyPointWithOrientation>& keypoints) {
    // Sort keypoints and remove duplicate keypoints
    //
    if (keypoints.size() < 2)
        return;
    
    std::set<KeyPointWithOrientation> st(keypoints.begin(), keypoints.end());

    keypoints = std::list(st.begin(), st.end());
}

// #############################
// # Keypoint scale conversion #
// #############################

void convertKeypointsToInputImageSize(std::list<KeyPointWithOrientation>& keypoints) {
    // Convert keypoint point, size, and octave to input image size
    //
    for (KeyPointWithOrientation& keypoint : keypoints) {
        keypoint.kp.pt *= 0.5f;
        keypoint.kp.size *= 0.5f; // Bug in original implementation
        keypoint.kp.octave = (keypoint.kp.octave & ~255) | ((keypoint.kp.octave - 1) & 255);
    }
}

// #########################
// # Descriptor generation #
// #########################

struct UnpackedKeypoint {
    size_t octave_plus_1;
    size_t layer;
    float scale;
};

UnpackedKeypoint unpackOctave(const KeyPoint& keypoint) {
    // Compute octave, layer, and scale from a keypoint
    //
    int octave = keypoint.octave & 255;
    int layer = (keypoint.octave >> 8) & 255;
    if (octave >= 128)
        octave = octave | -128;
    float scale = octave >= 0 ? 1.f / (float)(1 << octave) : (float)(1 << -octave);
    return UnpackedKeypoint{ integral_cast<size_t>(octave + 1), (size_t)layer, scale };
}

std::list<Array<float>> generateDescriptors(
    const std::list<KeyPointWithOrientation>& keypoints,
    const std::vector<std::vector<Array<float>>>& gaussian_images,
    size_t window_width=4,
    size_t num_bins=8,
    float scale_multiplier=3,
    float descriptor_max_value=0.2f)
{
    // Generate descriptors for each keypoint
    //
    // logger.debug('Generating descriptors...')
    std::list<Array<float>> descriptors;

    for (const KeyPointWithOrientation& keypoint : keypoints) {
        UnpackedKeypoint u = unpackOctave(keypoint.kp);
        const Array<float>& gaussian_image = gaussian_images[u.octave_plus_1][u.layer];
        FixedArray<size_t, 2> gshape = gaussian_image.fixed_shape<2>();
        FixedArray<ssize_t, 2> point = (u.scale * keypoint.kp.pt).applied<ssize_t>([](float v){return (ssize_t)std::round(v); });
        float bins_per_degree = (float)num_bins / 360.f;
        float angle = 360.f - keypoint.angle;
        float cos_angle = std::cos(deg2rad(angle));
        float sin_angle = std::sin(deg2rad(angle));
        float weight_multiplier = -0.5f / (squared(0.5f * (float)window_width));
        std::list<float> row_bin_list;
        std::list<float> col_bin_list;
        std::list<float> magnitude_list;
        std::list<float> orientation_bin_list;
        Array<float> histogram_tensor = zeros<float>(ArrayShape{ window_width + 2, window_width + 2, num_bins });   // first two dimensions are increased by 2 to account for border effects

        // Descriptor window size (described by half_width) follows OpenCV convention
        float hist_width = scale_multiplier * 0.5f * u.scale * keypoint.kp.size;
        int half_width = int(std::round(hist_width * std::sqrt(2.f) * float(window_width + 1) * 0.5f));   // sqrt(2) corresponds to diagonal length of a pixel
        half_width = int(std::min(half_width, (int)std::sqrt(squared(gshape(0)) + squared(gshape(1)))));     // ensure half_width lies within image

        for (ssize_t row = -half_width; row < half_width + 1; ++row) {
            for (ssize_t col = -half_width; col < half_width + 1; ++col) {
                float row_rot = (float)col * sin_angle + (float)row * cos_angle;
                float col_rot = (float)col * cos_angle - (float)row * sin_angle;
                float row_bin = (row_rot / hist_width) + 0.5f * (float)window_width - 0.5f;
                float col_bin = (col_rot / hist_width) + 0.5f * (float)window_width - 0.5f;
                if (row_bin > -1 && row_bin < (float)window_width && col_bin > -1 && col_bin < (float)window_width) {
                    int window_row = (int)(point(1) + row);
                    int window_col = (int)(point(0) + col);
                    if (window_row > 0 && window_row < (int)gshape(0) - 1 && window_col > 0 && window_col < (int)gshape(1) - 1) {
                        float dx = gaussian_image((size_t)window_row, (size_t)window_col + 1) - gaussian_image((size_t)window_row, (size_t)window_col - 1);
                        float dy = gaussian_image((size_t)window_row - 1, (size_t)window_col) - gaussian_image((size_t)window_row + 1, (size_t)window_col);
                        float gradient_magnitude = std::sqrt(dx * dx + dy * dy);
                        float gradient_orientation = std::fmod(rad2deg(std::atan2(dy, dx)), 360.f);
                        float weight = std::exp(weight_multiplier * (squared(row_rot / hist_width) + squared(col_rot / hist_width)));
                        row_bin_list.push_back(row_bin);
                        col_bin_list.push_back(col_bin);
                        magnitude_list.push_back(weight * gradient_magnitude);
                        orientation_bin_list.push_back((gradient_orientation - angle) * bins_per_degree);
                    }
                }
            }
        }

        auto row_bin = row_bin_list.begin();
        auto col_bin = col_bin_list.begin();
        auto magnitude = magnitude_list.begin();
        auto orientation_bin = orientation_bin_list.begin();
        for (size_t i = 0; i < row_bin_list.size(); ++i) {
            // Smoothing via trilinear interpolation
            // Notations follows https://en.wikipedia.org/wiki/Trilinear_interpolation
            // Note that we are really doing the inverse of trilinear interpolation here (we take the center value of the cube and distribute it among its eight neighbors)
            int row_bin_floor = (int)std::floor(*row_bin);
            int col_bin_floor = (int)std::floor(*col_bin);
            int orientation_bin_floor = (int)std::floor(*orientation_bin);
            float row_fraction = *row_bin - (float)row_bin_floor;
            float col_fraction = *col_bin - (float)col_bin_floor;
            float orientation_fraction = *orientation_bin - (float)orientation_bin_floor;
            if (orientation_bin_floor < 0)
                orientation_bin_floor += (int)num_bins;
            if (orientation_bin_floor >= (int)num_bins)
                orientation_bin_floor -= (int)num_bins;

            float c1 = *magnitude * row_fraction;
            float c0 = *magnitude * (1 - row_fraction);
            float c11 = c1 * col_fraction;
            float c10 = c1 * (1 - col_fraction);
            float c01 = c0 * col_fraction;
            float c00 = c0 * (1 - col_fraction);
            float c111 = c11 * orientation_fraction;
            float c110 = c11 * (1 - orientation_fraction);
            float c101 = c10 * orientation_fraction;
            float c100 = c10 * (1 - orientation_fraction);
            float c011 = c01 * orientation_fraction;
            float c010 = c01 * (1 - orientation_fraction);
            float c001 = c00 * orientation_fraction;
            float c000 = c00 * (1 - orientation_fraction);

            histogram_tensor((size_t)row_bin_floor + 1, (size_t)col_bin_floor + 1, (size_t)orientation_bin_floor) += c000;
            histogram_tensor((size_t)row_bin_floor + 1, (size_t)col_bin_floor + 1, ((size_t)orientation_bin_floor + 1) % num_bins) += c001;
            histogram_tensor((size_t)row_bin_floor + 1, (size_t)col_bin_floor + 2, (size_t)orientation_bin_floor) += c010;
            histogram_tensor((size_t)row_bin_floor + 1, (size_t)col_bin_floor + 2, ((size_t)orientation_bin_floor + 1) % num_bins) += c011;
            histogram_tensor((size_t)row_bin_floor + 2, (size_t)col_bin_floor + 1, (size_t)orientation_bin_floor) += c100;
            histogram_tensor((size_t)row_bin_floor + 2, (size_t)col_bin_floor + 1, ((size_t)orientation_bin_floor + 1) % num_bins) += c101;
            histogram_tensor((size_t)row_bin_floor + 2, (size_t)col_bin_floor + 2, (size_t)orientation_bin_floor) += c110;
            histogram_tensor((size_t)row_bin_floor + 2, (size_t)col_bin_floor + 2, ((size_t)orientation_bin_floor + 1) % num_bins) += c111;
            ++row_bin;
            ++col_bin;
            ++magnitude;
            ++orientation_bin;
        }

        Array<float> descriptor_vector{ ArrayShape{ histogram_tensor.shape(0) - 2, histogram_tensor.shape(1) - 2, histogram_tensor.shape(2) }};
        for (size_t r = 0; r < histogram_tensor.shape(0) - 2; ++r) {
            for (size_t c = 0; c < histogram_tensor.shape(1) - 2; ++c) {
                for (size_t h = 0; h < histogram_tensor.shape(2); ++h) {
                    descriptor_vector(r, c, h) = histogram_tensor(r + 1, c + 1, h);
                }
            }
        }
        descriptor_vector = descriptor_vector.flattened();  // Remove histogram borders
        // Threshold and normalize descriptor_vector
        float threshold = std::sqrt(sum(squared((descriptor_vector)))) * descriptor_max_value;
        descriptor_vector = minimum(descriptor_vector, threshold);
        descriptor_vector /= std::max(std::sqrt(sum(squared(descriptor_vector))), float_tolerance);
        // Multiply by 512, round, and saturate between 0 and 255 to convert from float32 to unsigned char (OpenCV convention)
        if (fac != 1.f) {
            descriptor_vector = (2.f * fac * descriptor_vector).applied([](float v){return std::round(v);});
            descriptor_vector = clipped(descriptor_vector, 0.f, fac);
        }
        descriptors.push_back(descriptor_vector);
    }
    return descriptors;
}

// #################
// # Main function #
// #################

SiftFeatures Mlib::Sift::computeKeypointsAndDescriptors(
    const Array<float>& image,
    float sigma,
    size_t num_intervals,
    float assumed_blur,
    size_t image_border_width)
{
    assert(image.ndim() == 2);
    // Compute SIFT keypoints and descriptors for an input image
    //
    Array<float> base_image = generateBaseImage(image, sigma, assumed_blur);
    size_t num_octaves = computeNumberOfOctaves(base_image.shape(0), base_image.shape(1));
    Array<float> gaussian_kernels = generateGaussianKernels(sigma, num_intervals);
    std::vector<std::vector<Array<float>>> gaussian_images = generateGaussianImages(base_image, num_octaves, gaussian_kernels);
    std::vector<std::vector<Array<float>>> dog_images = generateDoGImages(gaussian_images);
    std::list<KeyPointWithOrientation> keypoints = findScaleSpaceExtrema(gaussian_images, dog_images, num_intervals, sigma, image_border_width);
    removeDuplicateKeypoints(keypoints);
    convertKeypointsToInputImageSize(keypoints);
    std::list<Array<float>> descriptors = generateDescriptors(keypoints, gaussian_images);
    return SiftFeatures{ keypoints, descriptors };
}
