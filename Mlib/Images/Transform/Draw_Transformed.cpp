#include "Draw_Transformed.hpp"
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>
#include <Mlib/Images/Alpha_Channel_Mode.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Transform/Coefficient_Image.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <stdexcept>

using namespace Mlib;

class TransformedRoundImage {
public:
    TransformedRoundImage(
        const Array<float>& image,
        const TransformationMatrix<float, float, 2>& trafo)
        : image_(image)
        , trafo_{trafo}
        , itrafo_{trafo.inverted_scaled()}
    {
        if (image.ndim() != 3) {
            throw std::runtime_error("Image does not have 3 dimensions");
        }
        if (image.shape(1) != image.shape(2)) {
            throw std::runtime_error("Image is not quadratic");
        }
    }
    bool operator () (float rf, float cf, BilinearInterpolator<float>& bi) const {
        auto rcf = itrafo_.transform(FixedArray<float, 2>{rf, cf});
        return bilinear_interpolation(
            rcf(0) + integral_to_float<float>(image_.shape(1)) / 2.f,
            rcf(1) + integral_to_float<float>(image_.shape(2)) / 2.f,
            image_.shape(1),
            image_.shape(2),
            bi);
    }
    BoundingSphere<float, 2> bounding_sphere() const {
        return { trafo_.t, trafo_.get_scale() * integral_to_float<float>(image_.shape(1)) / 2.f };
    }
    AxisAlignedBoundingBox<float, 2> aabb() const {
        auto bs = bounding_sphere();
        return AxisAlignedBoundingBox<float, 2>::from_center_and_radius(
            bs.center, bs.radius);
    }
private:
    const Array<float> image_;
    TransformationMatrix<float, float, 2> trafo_;
    TransformationMatrix<float, float, 2> itrafo_;
};

void Mlib::draw_transformed(
    const Array<float>& image,
    Array<float>& canvas,
    const TransformationMatrix<float, float, 2>& trafo,
    AlphaChannelMode mode,
    CachedCoefficientImage* coeffs)
{
    if (image.ndim() != 3) {
        throw std::runtime_error("Image to be drawn does not have 3 dimensions");
    }
    if (canvas.ndim() != 3) {
        throw std::runtime_error("Canvas does not have 3 dimensions");
    }
    if (image.shape(0) != canvas.shape(0)) {
        throw std::runtime_error("Image channels differ from canvas channels");
    }
    if (image.shape(0) == 0) {
        return;
    }
    TransformedRoundImage transformed_round_image{image, trafo};
    std::vector<Array<float>> H(image.shape(0));
    for (size_t h = 0; h < canvas.shape(0); ++h) {
        H[h].ref() = image[h];
    }
    auto transformed_aabb = transformed_round_image.aabb();
    auto r_lower = float_to_integral<int>(std::floor(transformed_aabb.min(0)));
    auto r_upper = float_to_integral<int>(std::ceil(transformed_aabb.max(0)));
    #pragma omp parallel for
    for (int r = r_lower; r <= r_upper; ++r) {
        if (unhandled_exceptions_occured()) {
            continue;
        }
        try {
            float rf = integral_to_float<float>(r);
            for (float cf = std::floor(transformed_aabb.min(1)); cf <= std::ceil(transformed_aabb.max(1)); ++cf) {
                BilinearInterpolator<float> bi;
                if (transformed_round_image(rf, cf, bi)) {
                    size_t r = integral_cast<size_t>(positive_modulo(float_to_integral<int>(rf), integral_cast<int>(canvas.shape(1))));
                    size_t c = integral_cast<size_t>(positive_modulo(float_to_integral<int>(cf), integral_cast<int>(canvas.shape(2))));
                    switch (mode) {
                        case AlphaChannelMode::BLEND: {
                            float alpha_c = canvas(canvas.shape(0) - 1, r, c);
                            float alpha_i = bi(H[canvas.shape(0) - 1]);
                            float A = alpha_c;
                            float B = alpha_i;
                            float C = A + B - A * B;
                            canvas(canvas.shape(0) - 1, r, c) = C;
                            // For all x: (1-a)*(1-b)*x = (1-c)*x
                            // => (1-a)*(1-b) = (1-c)
                            // => c = a + b - ab
                            for (size_t h = 0; h < canvas.shape(0) - 1; ++h) {
                                canvas(h, r, c) = lerp(canvas(h, r, c) * alpha_c, bi(H[h]), alpha_i) / std::max(C, 1e-6f);
                            }
                            if (coeffs != nullptr) {
                                throw std::runtime_error("Coeffs are not implemented for blending");
                            }
                            continue;
                        }
                        case AlphaChannelMode::ADD: {
                            auto alpha = bi(H[canvas.shape(0) - 1]);
                            for (size_t h = 0; h < canvas.shape(0) - 1; ++h) {
                                canvas(h, r, c) += alpha * bi(H[h]);
                            }
                            canvas(canvas.shape(0) - 1, r, c) += alpha;
                            if (coeffs != nullptr) {
                                coeffs->add(r, c, {bi.r0, bi.c0, alpha * (1.f - bi.a0) * (1.f - bi.a1)});
                                coeffs->add(r, c, {bi.r0, bi.c1, alpha * (1.f - bi.a0) * bi.a1});
                                coeffs->add(r, c, {bi.r1, bi.c0, alpha * bi.a0 * (1.f - bi.a1)});
                                coeffs->add(r, c, {bi.r1, bi.c1, alpha * bi.a0 * bi.a1});
                            }
                            continue;
                        }
                        case AlphaChannelMode::OFF: {
                            for (size_t h = 0; h < canvas.shape(0); ++h) {
                                canvas(h, r, c) = bi(H[h]);
                            }
                            if (coeffs != nullptr) {
                                throw std::runtime_error("Coeffs are not implemented for alpha=off");
                            }
                            continue;
                        }
                    }
                    throw std::runtime_error("Unknown blend mode");
                }
            }
        } catch (...) {
            add_unhandled_exception(std::current_exception());
        }
    }
    if (unhandled_exceptions_occured()) {
        throw std::runtime_error("Unhandled exception(s) in Mlib::draw_transformed");
    }
}
