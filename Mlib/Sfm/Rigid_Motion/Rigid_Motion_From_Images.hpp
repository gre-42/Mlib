#pragma once
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Sampler.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Rodrigues.hpp>

namespace Mlib::Sfm::Rmfi {

template <class TData>
FixedArray<TData, 2> transform_coordinates(const TransformationMatrix<TData, TData, 2>& ki, const TransformationMatrix<TData, TData, 3>& ke, const FixedArray<TData, 2>& x, const TData& depth) {
    FixedArray<TData, 3, 3> iki{inv(ki.affine()).value()};
    FixedArray<TData, 3, 4> T{dot(ki.affine(), ke.semi_affine())};

    FixedArray<TData, 3> res3{dot(T, homogenized_4(dot(iki, homogenized_3(x)) * depth))};
    return FixedArray<TData, 2>{res3(0) / res3(2), res3(1) / res3(2)};
}

template <class TData>
FixedArray<TData, 2, 6> projected_points_jacobian_dke_1p_1ke_lifting(
    const FixedArray<TData, 2>& y,
    TData depth,
    const TransformationMatrix<TData, TData, 2>& ki_r,
    const TransformationMatrix<TData, TData, 2>& ki_l,
    const FixedArray<TData, 6>& kep)
{
    FixedArray<TData, 3> x = lstsq_chol_1d(ki_r.affine(), homogenized_3(y)).value() * depth;
    return Cv::projected_points_jacobian_dke_1p_1ke(x, ki_l, kep);
}

template <class TData>
Array<TData> d_pr_bilinear(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& im_r_depth,
    const TransformationMatrix<TData, TData, 2>& ki_r,
    const TransformationMatrix<TData, TData, 2>& ki_l,
    const TransformationMatrix<TData, TData, 3>& ke)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{ im_r.shape() };
    Cv::RigidMotionSampler rs{ki_r, ki_l, ke, im_r_depth, im_l.shape().erased_first()};
    #pragma omp parallel for
    for (int i = 0; i < (int)result.shape(1); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < result.shape(2); ++c) {
            BilinearInterpolator<TData> bi;
            if (!std::isnan(im_r_depth(r, c)) && rs.sample_destination(r, c, bi)) {
                for (size_t color = 0; color < im_r.shape(0); ++color) {
                    result(color, r, c) = bi(im_l, color) - im_r(color, r, c);
                }
            } else {
                for (size_t color = 0; color < im_r.shape(0); ++color) {
                    result(color, r, c) = NAN;
                }
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> intensity_jacobian(
    const Array<TData>& im_r_di,
    const Array<TData>& im_l_di,
    const Array<TData>& im_r_depth,
    const TransformationMatrix<TData, TData, 2>& ki_r,
    const TransformationMatrix<TData, TData, 2>& ki_l,
    const FixedArray<TData, 6>& kep)
{
    // Color, direction, row, column
    assert(im_r_di.ndim() == 4);
    assert(all(im_r_di.shape() == im_l_di.shape()));
    Array<TData> result{ArrayShape{ im_r_di.shape(0), im_r_di.shape(2), im_r_di.shape(3), 6 } };
    Cv::RigidMotionSampler hs{ki_r, ki_l, Cv::k_external(kep), im_r_depth, im_l_di.shape().erased_first()};
    #pragma omp parallel for
    for (int i = 0; i < (int)im_r_di.shape(2); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < im_r_di.shape(3); ++c) {
            if (std::isnan(im_r_depth(r, c))) {
                result[r][c] = NAN;
                continue;
            }
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<float, 2, 6> J = projected_points_jacobian_dke_1p_1ke_lifting(i2a(id_r), im_r_depth(r, c), ki_r, ki_l, kep);

            for (size_t color = 0; color < im_r_di.shape(0); ++color) {
                Array<float> im_grad{im_r_di(color, id1, r, c), im_r_di(color, id0, r, c)};
                BilinearInterpolator<TData> bi;
                if (hs.sample_destination(r, c, bi)) {
                    im_grad(0) = (im_grad(0) + bi(im_l_di, color, id1)) / 2;
                    im_grad(1) = (im_grad(1) + bi(im_l_di, color, id0)) / 2;
                }
                result[color][r][c] = dot(im_grad, J);
            }
        }
    }
    return result;
}

/**
 * im_r_di: Fixed difference image.
 * im_l_di: Moving difference image.
 */
template <class TData>
Array<TData> intensity_jacobian_fast(
    const Array<TData>& im_r_di,
    const Array<TData>& im_l_di,
    const Array<TData>& im_r_depth,
    const TransformationMatrix<TData, TData, 2>& ki_r,
    const TransformationMatrix<TData, TData, 2>& ki_l,
    const FixedArray<TData, 6>& kep)
{
    // Color, direction, row, column
    assert(im_r_di.ndim() == 4);
    assert(all(im_r_di.shape() == im_l_di.shape()));
    FixedArray<TData, 3, 3> iki_r{inv(ki_r.affine()).value()};
    FixedArray<TData, 3, 3> kif_l{ki_l.affine()};
    FixedArray<TData, 3, 4> ke{Cv::k_external(kep).semi_affine()};
    static const Array<TData> I = identity_array<TData>(3);
    // Changed order to be compatible with the rodrigues-implementation
    static const Array<TData> I0 = I[0];
    static const Array<TData> I1 = I[1];
    static const Array<TData> I2 = I[2];

    FixedArray<TData, 3, 3> R0{rodrigues2(I0, kep(0))};
    FixedArray<TData, 3, 3> R1{rodrigues2(I1, kep(1))};
    FixedArray<TData, 3, 3> R2{rodrigues2(I2, kep(2))};

    FixedArray<TData, 3, 3> cross0{cross(I0)};
    FixedArray<TData, 3, 3> cross1{cross(I1)};
    FixedArray<TData, 3, 3> cross2{cross(I2)};

    FixedArray<TData, 3, 3> RR2 = dot(R2, cross2);
    FixedArray<TData, 3, 3> RR1 = dot(R2, dot(R1, cross1));
    FixedArray<TData, 3, 3> RR0 = dot(R2, dot(R1, dot(R0, cross0)));

    Array<TData> result{ArrayShape{ im_r_di.shape(0), im_r_di.shape(2), im_r_di.shape(3), 6 } };

    Cv::RigidMotionSampler hs{ki_r, ki_l, Cv::k_external(kep), im_r_depth, im_l_di.shape().erased_first(2)};
    #pragma omp parallel for
    for (int i = 0; i < (int)im_r_di.shape(2); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < im_r_di.shape(3); ++c) {
            if (std::isnan(im_r_depth(r, c))) {
                for (size_t color = 0; color < im_r_di.shape(0); ++color) {
                    for (size_t i = 0; i < 6; ++i) {
                        result(color, r, c, i) = NAN;
                    }
                }
                continue;
            }
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<TData, 3> x = dot(iki_r, homogenized_3(i2a(id_r))) * im_r_depth(r, c);

            const auto m = kif_l.template row_range<0, 2>();
            const auto Mx = dot(kif_l, dot(ke, homogenized_4(x)));
            const auto mx = Mx.template row_range<0, 2>();
            const TData bx = Mx(2);

            const auto mx_2d = mx.template reshaped<2, 1>();
            const auto b_2d = kif_l.template row_range<2, 3>();

            FixedArray<TData, 2, 3> dy_da = ((m * bx) - dot(mx_2d, b_2d)) / squared(bx);

            FixedArray<TData, 3> r0 = dot(R0, x);
            FixedArray<TData, 3> r1 = dot(R1, r0);

            FixedArray<TData, 3, 3> da_dkep_T{
                dot(RR0, x),
                dot(RR1, r0),
                dot(RR2, r1) };

            auto da_dk = FixedArray<float, 3, 6>::init(
                da_dkep_T(0, 0), da_dkep_T(1, 0), da_dkep_T(2, 0), 1.f, 0.f, 0.f,
                da_dkep_T(0, 1), da_dkep_T(1, 1), da_dkep_T(2, 1), 0.f, 1.f, 0.f,
                da_dkep_T(0, 2), da_dkep_T(1, 2), da_dkep_T(2, 2), 0.f, 0.f, 1.f);

            FixedArray<TData, 2, 6> J = dot(dy_da, da_dk);

            for (size_t color = 0; color < im_r_di.shape(0); ++color) {
                FixedArray<TData, 2> im_grad{ im_r_di(color, id1, r, c), im_r_di(color, id0, r, c) };
                BilinearInterpolator<TData> bi;
                if (hs.sample_destination(r, c, bi)) {
                    im_grad(0) = (im_grad(0) + bi(im_l_di, color, id1)) / 2;
                    im_grad(1) = (im_grad(1) + bi(im_l_di, color, id0)) / 2;
                }
                FixedArray<TData, 6> intensity = dot(im_grad, J);
                for (size_t i = 0; i < 6; ++i) {
                    result(color, r, c, i) = intensity(i);
                }
            }
        }
    }
    return result;
}

template <class TData>
TransformationMatrix<TData, TData, 3> rigid_motion_from_images(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& im_r_depth,
    const TransformationMatrix<TData, TData, 2>& intrinsic_matrix_r,
    const TransformationMatrix<TData, TData, 2>& intrinsic_matrix_l,
    bool differentiate_numerically = false,
    const FixedArray<TData, 6>* x0 = nullptr,
    FixedArray<TData, 6>* xe = nullptr,
    bool print_residual = true)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    if (x0 != nullptr) {
        assert(x0->length() == 6);
    }
    const TData NAN_VALUE = 0;
    auto f = [&](const FixedArray<TData, 6>& x){
        return substitute_nans(d_pr_bilinear(im_r, im_l, im_r_depth, intrinsic_matrix_r, intrinsic_matrix_l, Cv::k_external(x)).flattened(), NAN_VALUE);
    };
    Array<TData> im_r_di = multichannel_central_gradient_filter(im_r);
    Array<TData> im_l_di = multichannel_central_gradient_filter(im_l);
    FixedArray<TData, 6> xx = levenberg_marquardt<TData, FixedArray<TData, 6, 6>, FixedArray<TData, 6>>(
        x0 == nullptr ? fixed_zeros<TData, 6>() : *x0,
        zeros<TData>(ArrayShape{im_r.nelements()}),
        f,
        [&](const FixedArray<TData, 6>& x){
            return differentiate_numerically
                ? mixed_numerical_differentiation(f, x)
                : substitute_nans(intensity_jacobian_fast(im_r_di, im_l_di, im_r_depth, intrinsic_matrix_r, intrinsic_matrix_l, x).rows_as_1D(), NAN_VALUE);
        },
        TData(1e-2),     // alpha,
        TData(1e-2),     // beta,
        TData(1e-2),     // alpha2,
        TData(1e-2),     // beta2,
        TData(0),        // min_redux
        200,             // niterations
        5,               // nburnin
        30,              // nmisses
        print_residual); // print_residual
    if (xe != nullptr) {
        *xe = xx;
    }
    return Cv::k_external(xx);
}

}
