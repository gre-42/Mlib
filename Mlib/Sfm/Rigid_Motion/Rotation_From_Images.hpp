#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
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
    const TransformationMatrix<TData, 2>& ki,
    const FixedArray<TData, 3>& theta)
{
    FixedArray<TData, 3, 3> R = tait_bryan_angles_2_matrix(theta);
    FixedArray<TData, 3> b = lstsq_chol_1d(ki.affine(), x);
    FixedArray<TData, 3> a = dot(R, b);
    FixedArray<TData, 2, 3> dy_da = homogeneous_jacobian_dx(ki.affine(), a);
    FixedArray<TData, 3, 3> da_dtheta = tait_bryan_angles_dtheta(theta, b);
    return dot2d(dy_da, da_dtheta);
}

template <class TData>
static FixedArray<TData, 3, 3> coordinate_transform(
    const FixedArray<TData, 3, 3>& R,
    const TransformationMatrix<float, 2>& intrinsic_matrix)
{
    return dot(dot(intrinsic_matrix.affine(), R), inv(intrinsic_matrix.affine()));
}

template <class TData>
static FixedArray<TData, 2> transform_coordinates(
    const FixedArray<TData, 3, 3>& R,
    const FixedArray<TData, 2>& x,
    const TransformationMatrix<float, 2>& intrinsic_matrix)
{
    FixedArray<TData, 3, 3> H = coordinate_transform(R, intrinsic_matrix);
    return apply_homography(H, x);
}

template <class TData>
Array<TData> d_pr(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    const FixedArray<TData, 3, 3>& R)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{im_r.shape()};
    FixedArray<TData, 3, 3> H = coordinate_transform(R, intrinsic_matrix);
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
    const TransformationMatrix<TData, 2>& intrinsic_matrix,
    const FixedArray<TData, 3, 3>& R)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{ im_r.shape() };
    ArrayShape space_shape{ im_r.shape(1), im_r.shape(2) };
    HomographySampler hs{coordinate_transform(R, intrinsic_matrix)};
    #pragma omp parallel for
    for (int i = 0; i < (int)result.shape(1); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < result.shape(2); ++c) {
            BilinearInterpolator<TData> bi;
            if (hs.sample_destination(r, c, space_shape, bi)) {
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
    const TransformationMatrix<TData, 2>& ki,
    const FixedArray<TData, 3>& theta)
{
    ArrayShape space_shape = ArrayShape{ im_r_di.shape(2), im_r_di.shape(3) };
    Array<TData> result{ArrayShape{ im_r_di.shape(0), im_r_di.shape(2), im_r_di.shape(3), 3 } };
    HomographySampler hs{coordinate_transform(tait_bryan_angles_2_matrix(theta), ki)};
    #pragma omp parallel for
    for (int i = 0; i < (int)im_r_di.shape(2); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < im_r_di.shape(3); ++c) {
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<float, 2, 3> J = projected_points_jacobian_dke_1p_1ke_only_rotation(homogenized_3(i2a(id_r)), ki, theta);

            for (size_t color = 0; color < im_r_di.shape(0); ++color) {
                FixedArray<float, 2> im_grad{im_r_di(color, id1, r, c), im_r_di(color, id0, r, c)};
                BilinearInterpolator<TData> bi;
                if (hs.sample_destination(r, c, space_shape, bi)) {
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
    const TransformationMatrix<TData, 2>& ki,
    const FixedArray<TData, 3>& theta)
{
    // Color, direction, row, column
    assert(im_r_di.ndim() == 4);
    assert(all(im_r_di.shape() == im_l_di.shape()));
    FixedArray<TData, 3, 3> Rd = tait_bryan_angles_2_matrix(theta);
    FixedArray<TData, 3, 3> R{Rd};
    FixedArray<TData, 3, 3> ki_inv{inv(ki.affine())};
    static const FixedArray<TData, 3, 3> I = fixed_identity_array<TData, 3>();
    // Changed order to be compatible with the rodrigues-implementation
    static const FixedArray<TData, 3> I0 = I[0];
    static const FixedArray<TData, 3> I1 = I[1];
    static const FixedArray<TData, 3> I2 = I[2];

    FixedArray<TData, 3, 3> R0{rodrigues(I0, theta(0))};
    FixedArray<TData, 3, 3> R1{rodrigues(I1, theta(1))};
    FixedArray<TData, 3, 3> R2{rodrigues(I2, theta(2))};

    FixedArray<TData, 3, 3> cross0{cross(I0)};
    FixedArray<TData, 3, 3> cross1{cross(I1)};
    FixedArray<TData, 3, 3> cross2{cross(I2)};

    FixedArray<TData, 3, 3> RR2 = dot(R2, cross2);
    FixedArray<TData, 3, 3> RR1 = dot(R2, dot(R1, cross1));
    FixedArray<TData, 3, 3> RR0 = dot(R2, dot(R1, dot(R0, cross0)));

    ArrayShape space_shape = ArrayShape{ im_r_di.shape(2), im_r_di.shape(3) };
    Array<TData> result{ArrayShape{ im_r_di.shape(0), im_r_di.shape(2), im_r_di.shape(3), 3 } };
    HomographySampler hs{coordinate_transform(R, ki)};
    const auto m = ki.affine().template row_range<0, 2>();
    const auto b_2d = ki.affine().template row_range<2, 3>();
    #pragma omp parallel for
    for (int i = 0; i < (int)im_r_di.shape(2); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < im_r_di.shape(3); ++c) {
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<TData, 3> x = homogenized_3(i2a(id_r));

            // projected_points_jacobian_dke_1p_1ke_only_rotation
            FixedArray<TData, 3> b = dot(ki_inv, x);
            FixedArray<TData, 3> a = dot(R, b);

            const auto Mx = dot(ki.affine(), a);
            const auto mx = Mx.template row_range<0, 2>();
            const float bx = Mx(2);

            const auto mx_2d = mx.template reshaped<2, 1>();

            FixedArray<TData, 2, 3> dy_da = ((m * bx) - dot(mx_2d, b_2d)) / squared(bx);

            FixedArray<TData, 3> r0 = dot(R0, b);
            FixedArray<TData, 3> r1 = dot(R1, r0);

            FixedArray<TData, 3, 3> da_dtheta_T;
            da_dtheta_T[0] = dot(RR0, b);
            da_dtheta_T[1] = dot(RR1, r0);
            da_dtheta_T[2] = dot(RR2, r1);

            FixedArray<TData, 2, 3> J = outer(dy_da, da_dtheta_T);

            for (size_t color = 0; color < im_r_di.shape(0); ++color) {
                FixedArray<TData, 2> im_grad{ im_r_di(color, id1, r, c), im_r_di(color, id0, r, c) };
                BilinearInterpolator<TData> bi;
                if (hs.sample_destination(r, c, space_shape, bi)) {
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
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    bool differentiate_numerically = false,
    FixedArray<TData, 3>* x0 = nullptr,
    FixedArray<TData, 3>* xe = nullptr,
    bool print_residual = true)
{
    assert(im_r.ndim() == 3);
    assert(all(im_r.shape() == im_l.shape()));
    const TData NAN_VALUE = 0;
    auto f = [&](const FixedArray<TData, 3>& x){
        return substitute_nans(d_pr_bilinear(im_r, im_l, intrinsic_matrix, tait_bryan_angles_2_matrix(x)).flattened(), NAN_VALUE);
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
                return intensity_jacobian_fast(im_r_di, im_l_di, intrinsic_matrix, x).rows_as_1D();
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
