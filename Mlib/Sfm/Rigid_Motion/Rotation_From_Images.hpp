#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_Sampler.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>

namespace Mlib::Sfm::Hfi {

/**
 * y = proj <- a = M x = R(theta) ki^{-1} x = R(theta) b
 * This function computes dy / dtheta,
 * by computing dy / da and da / dtheta.
 *
 * d/dtheta pi (ki R(theta) {ki^-1} x) =
 * d/dm Pi * ki * d/dtheta(R(theta) ki{^-1} x)
 * Jpi       Jk            Jr
 *
 * d/dtheta pi (K a) = d/dtheta proj (a)
 * See: projected_points_jacobian_dke_1p_1ke
 */
template <class TData>
FixedArray<TData, 2, 3> projected_points_jacobian_dke_1p_1ke_only_rotation(
    const FixedArray<TData, 3>& x,
    const TransformationMatrix<TData, TData, 2>& ki_r,
    const TransformationMatrix<TData, TData, 2>& ki_l,
    const FixedArray<TData, 3>& theta)
{
    FixedArray<TData, 3, 3> R = tait_bryan_angles_2_matrix(theta);
    FixedArray<TData, 3> b = lstsq_chol_1d(ki_r.affine(), x).value();
    FixedArray<TData, 3> a = dot(R, b);
    FixedArray<TData, 2, 3> dy_da = homogeneous_jacobian_dx(ki_l.affine(), a);
    FixedArray<TData, 3, 3> da_dtheta = tait_bryan_angles_dtheta(theta, b);
    return dot2d(dy_da, da_dtheta);
}

template <class TData>
static FixedArray<TData, 3, 3> coordinate_transform(
    const FixedArray<TData, 3, 3>& R,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_0,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_1)
{
    return dot2d(dot2d(intrinsic_matrix_1.affine(), R), inv(intrinsic_matrix_0.affine()).value());
}

template <class TData>
static FixedArray<TData, 2> transform_coordinates(
    const FixedArray<TData, 3, 3>& R,
    const FixedArray<TData, 2>& x,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_0,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_1)
{
    FixedArray<TData, 3, 3> H = coordinate_transform(R, intrinsic_matrix_0, intrinsic_matrix_1);
    return apply_homography(H, x);
}

template <class TData>
Array<TData> d_pr(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_0,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_1,
    const FixedArray<TData, 3, 3>& R)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{im_r.shape()};
    FixedArray<TData, 3, 3> H = coordinate_transform(R, intrinsic_matrix_0, intrinsic_matrix_1);
    FixedArray<size_t, 2> space_shape{ result.shape(1), result.shape(2) };
    #pragma omp parallel for
    for (int i = 0; i < (int)result.shape(1); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < result.shape(2); ++c) {
            FixedArray<size_t, 2> id_r(r, c);
            FixedArray<size_t, 2> id_l = a2i(apply_homography(H, i2a(id_r)));
            if (all(id_l < space_shape)) {
                for (size_t color = 0; color < result.shape(0); ++color) {
                    result(color, r, c) = im_l(color, id_l(0), id_l(1)) - im_r(color, id_r(0), id_r(1));
                }
            } else {
                for (size_t color = 0; color < result.shape(0); ++color) {
                    result(color, r, c) = NAN;
                }
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> d_pr_bilinear(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const TransformationMatrix<TData, TData, 2>& intrinsic_matrix_r,
    const TransformationMatrix<TData, TData, 2>& intrinsic_matrix_l,
    const FixedArray<TData, 3, 3>& R)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{ im_r.shape() };
    HomographySampler hs{coordinate_transform(R, intrinsic_matrix_r, intrinsic_matrix_l)};
    #pragma omp parallel for
    for (int i = 0; i < (int)result.shape(1); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < result.shape(2); ++c) {
            BilinearInterpolator<TData> bi;
            if (hs.sample_destination(r, c, im_r.shape(1), im_r.shape(2), bi)) {
                for (size_t color = 0; color < result.shape(0); ++color) {
                    result(color, r, c) = bi(im_l, color) - im_r(color, r, c);
                }
            } else {
                for (size_t color = 0; color < result.shape(0); ++color) {
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
    const TransformationMatrix<TData, TData, 2>& ki_r,
    const TransformationMatrix<TData, TData, 2>& ki_l,
    const FixedArray<TData, 3>& theta)
{
    Array<TData> result{ArrayShape{ im_r_di.shape(0), im_r_di.shape(2), im_r_di.shape(3), 3 } };
    HomographySampler hs{coordinate_transform(tait_bryan_angles_2_matrix(theta), ki_r, ki_l)};
    #pragma omp parallel for
    for (int i = 0; i < (int)im_r_di.shape(2); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < im_r_di.shape(3); ++c) {
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<float, 2, 3> J = projected_points_jacobian_dke_1p_1ke_only_rotation(homogenized_3(i2a(id_r)), ki_r, ki_l, theta);

            for (size_t color = 0; color < im_r_di.shape(0); ++color) {
                FixedArray<float, 2> im_grad{im_r_di(color, id1, r, c), im_r_di(color, id0, r, c)};
                BilinearInterpolator<TData> bi;
                if (hs.sample_destination(r, c, im_r_di.shape(2), im_r_di.shape(3), bi)) {
                    im_grad(0) = (im_grad(0) + bi(im_l_di, color, id1)) / 2;
                    im_grad(1) = (im_grad(1) + bi(im_l_di, color, id0)) / 2;
                }
                FixedArray<TData, 3> intensity = dot(im_grad, J);
                for (size_t i = 0; i < 3; ++i) {
                    result(color, r, c, i) = intensity(i);
                }
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> intensity_jacobian_fast(
    const Array<TData>& im_r_di,
    const Array<TData>& im_l_di,
    const TransformationMatrix<TData, TData, 2>& ki_r,
    const TransformationMatrix<TData, TData, 2>& ki_l,
    const FixedArray<TData, 3>& theta)
{
    // Color, direction, row, column
    assert(im_r_di.ndim() == 4);
    assert(all(im_r_di.shape() == im_l_di.shape()));
    FixedArray<TData, 3, 3> Rd = tait_bryan_angles_2_matrix(theta);
    FixedArray<TData, 3, 3> R{Rd};
    FixedArray<TData, 3, 3> ki_r_inv{inv(ki_r.affine()).value()};
    static const FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    // Changed order to be compatible with the rodrigues-implementation
    static const FixedArray<TData, 3> I0 = I[0];
    static const FixedArray<TData, 3> I1 = I[1];
    static const FixedArray<TData, 3> I2 = I[2];

    FixedArray<TData, 3, 3> R0{rodrigues2(I0, theta(0))};
    FixedArray<TData, 3, 3> R1{rodrigues2(I1, theta(1))};
    FixedArray<TData, 3, 3> R2{rodrigues2(I2, theta(2))};

    FixedArray<TData, 3, 3> cross0{cross(I0)};
    FixedArray<TData, 3, 3> cross1{cross(I1)};
    FixedArray<TData, 3, 3> cross2{cross(I2)};

    FixedArray<TData, 3, 3> RR2 = dot2d(R2, cross2);
    FixedArray<TData, 3, 3> RR1 = dot2d(R2, dot2d(R1, cross1));
    FixedArray<TData, 3, 3> RR0 = dot2d(R2, dot2d(R1, dot2d(R0, cross0)));

    Array<TData> result{ArrayShape{ im_r_di.shape(0), im_r_di.shape(2), im_r_di.shape(3), 3 } };
    HomographySampler hs{coordinate_transform(R, ki_r, ki_l)};
    const auto m = ki_l.affine().template row_range<0, 2>();
    const auto b_2d = ki_l.affine().template row_range<2, 3>();
    #pragma omp parallel for
    for (int i = 0; i < (int)im_r_di.shape(2); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < im_r_di.shape(3); ++c) {
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<TData, 3> x = homogenized_3(i2a(id_r));

            // projected_points_jacobian_dke_1p_1ke_only_rotation
            FixedArray<TData, 3> b = dot1d(ki_r_inv, x);
            FixedArray<TData, 3> a = dot1d(R, b);

            const auto Mx = dot1d(ki_l.affine(), a);
            const auto mx = Mx.template row_range<0, 2>();
            const float bx = Mx(2);

            const auto mx_2d = mx.template reshaped<2, 1>();

            FixedArray<TData, 2, 3> dy_da = ((m * bx) - dot2d(mx_2d, b_2d)) / squared(bx);

            FixedArray<TData, 3> r0 = dot1d(R0, b);
            FixedArray<TData, 3> r1 = dot1d(R1, r0);

            FixedArray<TData, 3, 3> da_dtheta_T{
                dot1d(RR0, b),
                dot1d(RR1, r0),
                dot1d(RR2, r1) };

            FixedArray<TData, 2, 3> J = outer(dy_da, da_dtheta_T);

            for (size_t color = 0; color < im_r_di.shape(0); ++color) {
                FixedArray<TData, 2> im_grad{ im_r_di(color, id1, r, c), im_r_di(color, id0, r, c) };
                BilinearInterpolator<TData> bi;
                if (hs.sample_destination(r, c, im_r_di.shape(2), im_r_di.shape(3), bi)) {
                    im_grad(0) = (im_grad(0) + bi(im_l_di, color, id1)) / 2;
                    im_grad(1) = (im_grad(1) + bi(im_l_di, color, id0)) / 2;
                }
                FixedArray<TData, 3> intensity = dot(im_grad, J);
                for (size_t i = 0; i < 3; ++i) {
                    result(color, r, c, i) = intensity(i);
                }
            }
        }
    }
    return result;
}

template <class TData>
FixedArray<TData, 3, 3> rotation_from_images(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_r,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_l,
    bool differentiate_numerically = false,
    FixedArray<TData, 3>* x0 = nullptr,
    FixedArray<TData, 3>* xe = nullptr,
    bool print_residual = true)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    const TData NAN_VALUE = 0;
    auto f = [&](const FixedArray<TData, 3>& x){
        return substitute_nans(d_pr_bilinear(im_r, im_l, intrinsic_matrix_r, intrinsic_matrix_l, tait_bryan_angles_2_matrix(x)).flattened(), NAN_VALUE);
    };
    Array<float> im_r_di = multichannel_central_gradient_filter(im_r);
    Array<float> im_l_di = multichannel_central_gradient_filter(im_l);
    FixedArray<TData, 3> xx = levenberg_marquardt<TData, FixedArray<float, 3, 3>, FixedArray<float, 3>>(
        x0 != nullptr ? *x0 : fixed_zeros<TData, 3>(),
        zeros<TData>(ArrayShape{im_r.nelements()}),
        f,
        [&](const FixedArray<TData, 3>& x){
            if (differentiate_numerically) {
                return mixed_numerical_differentiation(f, x);
            } else {
                return intensity_jacobian_fast(im_r_di, im_l_di, intrinsic_matrix_r, intrinsic_matrix_l, x).rows_as_1D();
            }
        },
        TData(1e-2),     // alpha,
        TData(1e-2),     // beta,
        TData(1e-2),     // alpha2,
        TData(1e-2),     // beta2,
        TData(0),        // min_redux
        100,             // niterations
        5,               // nburnin
        3,               // nmisses
        print_residual); // print_residual
    if (xe != nullptr) {
        *xe = xx;
    }
    return FixedArray<float, 3, 3>{ tait_bryan_angles_2_matrix(xx) };
}

}
