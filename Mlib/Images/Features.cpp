#include "Features.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Draw_Generic.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Sort.hpp>

using namespace Mlib;

void Mlib::hessian_determinant_trace(
    const Array<float>& image,
    Array<float>* determinant,
    Array<float>* trace)
{
    assert(image.ndim() == 2);
    if (determinant != nullptr) {
        *determinant = zeros<float>(image.shape());
    }
    if (trace != nullptr) {
        *trace = zeros<float>(image.shape());
    }
    Array<float> hessian(ArrayShape{2, 2});
    for (size_t r = 1; r + 1 < image.shape(0); r++) {
        for (size_t c = 1; c + 1 < image.shape(1); c++) {
            //off_derivatives(0, 0) = (image[r + 1](c - 1) - image[r - 1](c - 1)) / 2.0;
            //off_derivatives(0, 1) = (image[r + 1](c + 1) - image[r - 1](c + 1)) / 2.0;
            //off_derivatives(1, 0) = (image[r - 1](c + 1) - image[r - 1](c - 1)) / 2.0;
            //off_derivatives(1, 1) = (image[r + 1](c + 1) - image[r + 1](c - 1)) / 2.0;
            // d00 d01
            // d10 d11
            hessian(0, 0) = (image[r - 1](c) - 2.f * image(r, c) + image[r + 1](c)) / 4.f;
            hessian(1, 1) = (image[r](c - 1) - 2.f * image(r, c) + image[r](c + 1)) / 4.f;
            hessian(0, 1) = (
                (image[r + 1](c + 1) - image[r - 1](c + 1)) -
                (image[r + 1](c - 1) - image[r - 1](c - 1))) / 4.f;
            hessian(1, 0) = (
                (image[r + 1](c + 1) - image[r + 1](c - 1)) -
                (image[r - 1](c + 1) - image[r - 1](c - 1))) / 4.f;
            //Array<float> q;
            //Array<float> s;
            //qdq(hessian, q, s);
            //if (s(0) * s(1) < -delta) {
            if (determinant != nullptr) {
                (*determinant)(r, c) = det2x2(hessian);
            }
            if (trace != nullptr) {
                (*trace)(r, c) = trace2x2(hessian);
            }
        }
    }
}

Array<float> Mlib::find_saddle_points(
    const Array<float>& image,
    float delta)
{
    assert(image.ndim() == 2);
    std::list<Array<float>> feature_points;

    Array<float> det;
    hessian_determinant_trace(image, &det, nullptr);
    for (size_t r = 1; r + 1 < image.shape(0); r++) {
        for (size_t c = 1; c + 1 < image.shape(1); c++) {
            if (det(r, c) < delta) {
                feature_points.push_back(i2a(ArrayShape{r, c}));
            }
        }
    }
    return Array<float>(feature_points, ArrayShape{0, 2});
}

Array<float> Mlib::structure_tensor(
    const Array<float>& image,
    Array<float>* det,
    Array<float>* trace)
{
    assert(image.ndim() == 2);
    auto Ix = central_differences_1d(image, 1).flattened();
    auto Iy = central_differences_1d(image, 0).flattened();
    Array<float> M(ArrayShape{2, 2}.concatenated(image.shape()));
    size_t nelements = image.nelements();
    Array<float> M_f = M.reshaped(ArrayShape{2, 2, nelements});
    for (size_t i = 0; i < nelements; ++i) {
        M_f(0, 0, i) = squared(Ix(i));
        M_f(0, 1, i) = Ix(i) * Iy(i);
        M_f(1, 0, i) = Iy(i) * Ix(i);
        M_f(1, 1, i) = squared(Iy(i));
    }
    if (det != nullptr) {
        *det = det2x2_a(M_f).reshaped(image.shape());
    }
    if (trace != nullptr) {
        *trace = trace2x2_a(M_f).reshaped(image.shape());
    }
    return M;
}

Array<float> Mlib::harris_response(
    const Array<float>& image,
    Array<bool>* feature_mask)
{
    Array<float> M_det;
    Array<float> M_trace;
    structure_tensor(image, &M_det, &M_trace);
    if (feature_mask != nullptr) {
        *feature_mask = (abs(M_trace) < 0.01f);
    }
    return M_det - 0.05f * squared(M_trace);
}

Array<float> Mlib::find_harris_corners(const Array<float>& image, float threshold) {
    std::list<Array<float>> result;
    Array<float> hr = harris_response(image);
    Array<bool> maxima = find_local_maxima(-hr, false);
    image.shape().foreach([&](const ArrayShape& index) {
        if (maxima(index) && (hr(index) < threshold)) {
            // Adding 0.5 selects the center of the pixel
            // Reverting to swap rows/cols
            result.push_back(i2a(index));
        }
    });
    return Array<float>(result, ArrayShape{0, 2});
}

Array<float> Mlib::find_nfeatures(
    const Array<float>& featureness,
    const Array<bool>& mask,
    size_t nfeatures,
    float distance_sigma)
{
    assert(all(mask.shape() == featureness.shape()));
    Array<bool> masked_maxima = find_local_maxima(gaussian_filter_NWE(featureness, distance_sigma, NAN), false) && mask && (!Mlib::isnan(featureness));

    // negate to reverse sort-order
    Array<float> sorted_negated_featureness = -featureness[masked_maxima];
    size_t nfeatures_corrected = std::min(count_nonzero(masked_maxima), nfeatures);
    Array<float> result(ArrayShape{nfeatures_corrected, featureness.ndim()});
    if (nfeatures_corrected > 0) {
        sort(sorted_negated_featureness);
        float threshold = -sorted_negated_featureness(nfeatures_corrected - 1);
        size_t r = 0;
        featureness.shape().foreach([&](const ArrayShape& index){
            if (masked_maxima(index) &&
                (featureness(index) >= threshold) && // support ties (1)
                (r < result.shape(0))) // support ties (2)
            {
                result[r] = i2a(index);
                ++r;
            }
        });
        assert(r == result.shape(0));
    }
    return result;
}

void Mlib::highlight_features(
    const Array<float>& feature_points,
    PpmImage& bitmap,
    size_t size,
    const Rgb24& color)
{
    draw_points_as_boxes(feature_points, bitmap, size, color);
}

Array<float> Mlib::find_feature_in_neighborhood(
    const Array<float>& old_feature_point,
    const Array<float>& new_featureness,
    const ArrayShape& window_shape)
{
    assert(all(old_feature_point.shape() == ArrayShape{2}));
    assert(new_featureness.ndim() == 2);
    assert(window_shape.ndim() == 2);

    ArrayShape index{a2i(old_feature_point)};
    ArrayShape best_id(old_feature_point.length());
    best_id = SIZE_MAX;
    float best_featureness = -std::numeric_limits<float>::infinity();
    for (size_t r = 0; r < window_shape(0); ++r) {
        for (size_t c = 0; c < window_shape(1); ++c) {
            size_t R = r + index(0);
            size_t C = c + index(1);
            if ((R < new_featureness.shape(0)) &&
                (C < new_featureness.shape(1)))
            {
                float f = new_featureness(R, C);
                if (f > best_featureness) {
                    best_id(0) = R;
                    best_id(1) = C;
                    best_featureness = f;
                }
            }
        }
    }
    return i2a(best_id);
}
