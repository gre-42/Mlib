#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_Sampler.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>

namespace Mlib { namespace Sfm { namespace Hfi {

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
Array<TData> projected_points_jacobian_dke_1p_1ke_only_rotation(
    const Array<TData>& x,
    const Array<TData>& ki,
    const Array<TData>& theta)
{
    assert(x.length() == 3);
    assert(all(ki.shape() == ArrayShape{3, 3}));
    assert(theta.length() == 3);
    Array<TData> R = tait_bryan_angles_2_matrix(theta);
    Array<TData> b = lstsq_chol_1d(ki, x);
    Array<TData> a = dot(R, b);
    Array<TData> dy_da = homogeneous_jacobian_dx(ki, a);
    Array<TData> da_dtheta = tait_bryan_angles_dtheta(theta, b);
    return dot(dy_da, da_dtheta);
}

template <class TData>
static Array<TData> coordinate_transform(
    const Array<TData>& R,
    const Array<TData>& intrinsic_matrix)
{
    return dot(dot(intrinsic_matrix, R), inv(intrinsic_matrix));
}

template <class TData>
static Array<TData> transform_coordinates(
    const Array<TData>& R,
    const Array<TData>& x,
    const Array<TData>& intrinsic_matrix)
{
    Array<TData> H = coordinate_transform(R, intrinsic_matrix);
    return dehomogenized_2(apply_homography(H, homogenized_3(x)));
}

template <class TData>
Array<TData> d_pr(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& intrinsic_matrix,
    const Array<TData>& R)
{
    assert(im_r.ndim() == 2);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{im_r.shape()};
    Array<TData> H_d = coordinate_transform(R, intrinsic_matrix);
    FixedArray<TData, 3, 3> H{H_d};
    FixedArray<size_t, 2> shape{result.shape()};
    #pragma omp parallel for
    for(size_t r = 0; r < result.shape(0); ++r) {
        for(size_t c = 0; c < result.shape(1); ++c) {
            FixedArray<size_t, 2> id_r(r, c);
            FixedArray<size_t, 2> id_l = a2i(dehomogenized_2(apply_homography(H, homogenized_3(i2a(id_r)))));
            if (all(id_l < shape)) {
                result(r, c) = im_l(id_l(0), id_l(1)) - im_r(id_r(0), id_r(1));
            } else {
                result(r, c) = NAN;
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> d_pr_bilinear(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& intrinsic_matrix,
    const Array<TData>& R)
{
    assert(im_r.ndim() == 2);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{im_r.shape()};
    FixedArray<size_t, 2> shape{result.shape()};
    HomographySampler hs{coordinate_transform(R, intrinsic_matrix)};
    #pragma omp parallel for
    for(size_t r = 0; r < result.shape(0); ++r) {
        for(size_t c = 0; c < result.shape(1); ++c) {
            BilinearInterpolator<TData> bi;
            if (hs.sample_destination(r, c, im_l.shape(), bi)) {
                result(r, c) = bi.interpolate_grayscale(im_l) - im_r(r, c);
            } else {
                result(r, c) = NAN;
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> intensity_jacobian(
    const Array<TData>& im_r_di,
    const Array<TData>& im_l_di,
    const Array<TData>& ki,
    const Array<TData>& theta)
{
    ArrayShape space_shape = im_r_di.shape().erased_first();
    Array<TData> result{space_shape.appended(3)};
    HomographySampler hs{coordinate_transform(tait_bryan_angles_2_matrix(theta), ki)};
    #pragma omp parallel for
    for(size_t r = 0; r < im_r_di.shape(1); ++r) {
        for(size_t c = 0; c < im_r_di.shape(2); ++c) {
            ArrayShape id_r{r, c};
            Array<float> J = projected_points_jacobian_dke_1p_1ke_only_rotation(homogenized_3(i2a(id_r)), ki, theta);
            Array<float> im_grad{im_r_di(id1, r, c), im_r_di(id0, r, c)};
            BilinearInterpolator<TData> bi;
            if (hs.sample_destination(r, c, space_shape, bi)) {
                im_grad(0) = (im_grad(0) + bi.interpolate_multichannel(im_l_di, id1)) / 2;
                im_grad(1) = (im_grad(1) + bi.interpolate_multichannel(im_l_di, id0)) / 2;
            }
            result[r][c] = dot(im_grad, J);
        }
    }
    return result;
}

template <class TData>
Array<TData> intensity_jacobian_fast(
    const Array<TData>& im_r_di,
    const Array<TData>& im_l_di,
    const Array<TData>& ki,
    const Array<TData>& theta)
{
    Array<TData> Rd = tait_bryan_angles_2_matrix(theta);
    FixedArray<TData, 3, 3> kif{ki};
    FixedArray<TData, 3, 3> R{Rd};
    FixedArray<TData, 3, 3> ki_inv{inv(ki)};
    static const Array<TData> I = identity_array<TData>(3);
    // Changed order to be compatible with the rodrigues-implementation
    static const Array<TData> I0 = I[0];
    static const Array<TData> I1 = I[1];
    static const Array<TData> I2 = I[2];

    FixedArray<TData, 3, 3> R0{rodrigues(I0, theta(0))};
    FixedArray<TData, 3, 3> R1{rodrigues(I1, theta(1))};
    FixedArray<TData, 3, 3> R2{rodrigues(I2, theta(2))};

    FixedArray<TData, 3, 3> cross0{cross(I0)};
    FixedArray<TData, 3, 3> cross1{cross(I1)};
    FixedArray<TData, 3, 3> cross2{cross(I2)};

    FixedArray<TData, 3, 3> RR2 = dot(R2, cross2);
    FixedArray<TData, 3, 3> RR1 = dot(R2, dot(R1, cross1));
    FixedArray<TData, 3, 3> RR0 = dot(R2, dot(R1, dot(R0, cross0)));

    ArrayShape space_shape = im_r_di.shape().erased_first();
    Array<TData> result{space_shape.appended(3)};
    HomographySampler hs{coordinate_transform(Rd, ki)};
    #pragma omp parallel for
    for(size_t r = 0; r < im_r_di.shape(1); ++r) {
        for(size_t c = 0; c < im_r_di.shape(2); ++c) {
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<TData, 3> x = homogenized_3(i2a(id_r));

            // projected_points_jacobian_dke_1p_1ke_only_rotation
            assert(x.length() == 3);
            assert(all(ki.shape() == ArrayShape{3, 3}));
            assert(theta.length() == 3);
            FixedArray<TData, 3> b = dot(ki_inv, x);
            FixedArray<TData, 3> a = dot(R, b);

            const auto m = kif.template row_range<0, 2>();
            const auto Mx = dot(kif, a);
            const auto mx = Mx.template row_range<0, 2>();
            const float bx = Mx(2);

            const auto mx_2d = mx.template reshaped<2, 1>();
            const auto b_2d = kif.template row_range<2, 3>();

            FixedArray<TData, 2, 3> dy_da = ((m * bx) - dot(mx_2d, b_2d)) / squared(bx);

            FixedArray<TData, 3> r0 = dot(R0, b);
            FixedArray<TData, 3> r1 = dot(R1, r0);

            FixedArray<TData, 3, 3> da_dtheta_T;
            da_dtheta_T[0] = dot(RR0, b);
            da_dtheta_T[1] = dot(RR1, r0);
            da_dtheta_T[2] = dot(RR2, r1);

            FixedArray<TData, 2, 3> J = outer(dy_da, da_dtheta_T);

            FixedArray<TData, 2> im_grad{im_r_di(id1, r, c), im_r_di(id0, r, c)};
            BilinearInterpolator<TData> bi;
            if (hs.sample_destination(r, c, space_shape, bi)) {
                im_grad(0) = (im_grad(0) + bi.interpolate_multichannel(im_l_di, id1)) / 2;
                im_grad(1) = (im_grad(1) + bi.interpolate_multichannel(im_l_di, id0)) / 2;
            }
            FixedArray<TData, 3> intensity = dot(im_grad, J);
            for(size_t i = 0; i < 3; ++i) {
                result(r, c, i) = intensity(i);
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> rotation_from_images(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& intrinsic_matrix,
    bool differentiate_numerically = false,
    Array<TData>* x0 = nullptr,
    Array<TData>* xe = nullptr)
{
    assert(im_r.ndim() == 2);
    assert(all(im_r.shape() == im_l.shape()));
    if (x0 != nullptr) {
        assert(x0->length() == 3);
    }
    const TData NAN_VALUE = 0;
    auto f = [&](const Array<TData>& x){
        return substitute_nans(d_pr(im_r, im_l, intrinsic_matrix, tait_bryan_angles_2_matrix(x)).flattened(), NAN_VALUE);
    };
    Array<float> im_r_di = central_gradient_filter(im_r);
    Array<float> im_l_di = central_gradient_filter(im_l);
    Array<TData> xx = levenberg_marquardt(
        x0 != nullptr ? *x0 : zeros<TData>(ArrayShape{3}),
        zeros<TData>(ArrayShape{im_r.nelements()}),
        f,
        [&](const Array<TData>& x){
            if (differentiate_numerically) {
                return numerical_differentiation(f, x);
            } else {
                return intensity_jacobian_fast(im_r_di, im_l_di, intrinsic_matrix, x).rows_as_1D();
            }
        },
        TData(1e-2),   // alpha,
        TData(1e-2),   // beta,
        TData(1e-2),   // alpha2,
        TData(1e-2),   // beta2,
        TData(0),      // min_redux
        100,           // niterations
        5,             // nburnin
        3);            // nmisses
    if (xe != nullptr) {
        *xe = xx;
    }
    return tait_bryan_angles_2_matrix(xx);
}

}}}
