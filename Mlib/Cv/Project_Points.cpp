#include "Project_Points.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Math/Svd4.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

TransformationMatrix<float, float, 3> Mlib::Cv::k_external(const FixedArray<float, 6>& kep) {
    // auto ro = rodrigues1<float>(Array<float>{k(0), k(1), k(2)});
    auto ro = tait_bryan_angles_2_matrix(FixedArray<float, 3>{kep(0), kep(1), kep(2)});
    return TransformationMatrix<float, float, 3>(ro, FixedArray<float, 3>{kep(3), kep(4), kep(5)});
}

FixedArray<float, 6> Mlib::Cv::k_external_inverse(const TransformationMatrix<float, float, 3>& ke) {
    FixedArray<float, 3> angles = matrix_2_tait_bryan_angles(ke.R);
    return FixedArray<float, 6>{
        angles(0), angles(1), angles(2),
        ke.t(0), ke.t(1), ke.t(2)};
}

TransformationMatrix<float, float, 2> Mlib::Cv::k_internal(const FixedArray<float, 4>& kip) {
    assert(kip.length() == 4);
    return TransformationMatrix<float, float, 2>{
        FixedArray<float, 2, 2>::init(
            kip(0), 0.f,
            0.f, kip(2)),
        FixedArray<float, 2>{kip(1), kip(3)}};
}

FixedArray<float, 4> Mlib::Cv::pack_k_internal(const TransformationMatrix<float, float, 2>& ki) {
    FixedArray<float, 4> kip{ki.R(0, 0), ki.t(0), ki.R(1, 1), ki.t(1)};
    assert(all(k_internal(kip).semi_affine() == ki.semi_affine()));
    return kip;
}

Array<FixedArray<float, 2>> Mlib::Cv::projected_points(
    const Array<FixedArray<float, 3>>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<TransformationMatrix<float, float, 3>>& ke,
    PointAtInfinityBehavior point_at_infinity_behavior)
{
    assert(x.ndim() == 1);
    assert(ke.ndim() == 1);
    // lerr() << x.shape() << " | " << ki.shape() << " | " << ke.shape();
    Array<FixedArray<float, 2>> y{ ArrayShape{ke.length(), x.length()} };
    for (size_t i = 0; i < ke.shape(0); ++i) {
        // lerr() << ki.shape() << " | " << ke[i].shape();
        auto m = TransformationMatrix<float, float, 3>{ ki.project(ke(i).semi_affine()) };
        // auto kk = R3_from_Nx4(ke[i], 3);
        // lerr() << (kk.T(), kk.T());
        // lerr() << (kk, kk);
        // lerr() << ki;
        // lerr() << m.shape() << " - " << p.shape();
        // lerr() << p[i].shape() << " . " << (m, P).T().shape();
        for (size_t j = 0; j < x.length(); ++j) {
            FixedArray<float, 3> yy = m.transform(x(j));

            if (std::isnan(yy(2))) {
                throw std::runtime_error("projected_points: t is NaN");
            }
            if (std::abs(yy(2)) < 1e-6) {
                if (point_at_infinity_behavior == PointAtInfinityBehavior::IS_NAN) {
                    y(i, j) = NAN;
                } else {
                    throw std::runtime_error("projected_points: t too small: " + std::to_string(yy(2)));
                }
            } else {
                y(i, j)(0) = yy(0) / yy(2);
                y(i, j)(1) = yy(1) / yy(2);
            }
        }
    }
    // lerr() << "y\n" << y;
    return y;
}

Array<FixedArray<float, 2>> Mlib::Cv::projected_points_1ke(
    const Array<FixedArray<float, 3>>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke,
    PointAtInfinityBehavior point_at_infinity_behavior)
{
    assert(x.ndim() == 1);
    Array<FixedArray<float, 2>> res = projected_points(
        x,
        ki,
        Array<TransformationMatrix<float, float, 3>>{ ke },
        point_at_infinity_behavior);
    assert(all(res.shape() == ArrayShape{1, x.length()}));
    return res[0];
}

FixedArray<float, 2> Mlib::Cv::projected_points_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke,
    PointAtInfinityBehavior point_at_infinity_behavior)
{
    Array<FixedArray<float, 2>> res = projected_points_1ke(
        Array<FixedArray<float, 3>>{ x },
        ki,
        ke,
        point_at_infinity_behavior);
    assert(all(res.shape() == ArrayShape{1}));
    return res(0);
}

/*
 * y = proj <- a <- K x
 * This function computes dy / dx.
 */
FixedArray<float, 2, 3> Mlib::Cv::projected_points_jacobian_dx_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke)
{
    return homogeneous_jacobian_dx(TransformationMatrix<float, float, 3>{ ki.project(ke.semi_affine()) }, x);
}

/*
 * y = proj <- a <- K x
 * This function computes dy / dk,
 * by computing dy / da and da / dk.
 *
 * See: projected_points_jacobian_dke_1p_1ke_only_rotation
 */
FixedArray<float, 2, 6> Mlib::Cv::projected_points_jacobian_dke_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 2>& ki,
    const FixedArray<float, 6>& kep)
{
    assert(kep.length() == 6);
    TransformationMatrix<float, float, 3> ke = k_external(kep);
    FixedArray<float, 2, 3> dy_da = homogeneous_jacobian_dx(ki.affine(), ke.transform(x));
    // Array<float> da_dkep = rodrigues_jacobian_dk(kep.row_range(0, 3), x.row_range(0, 3));

    // Numerical differentiation
    // Array<float> da_dkep = numerical_differentiation([&](
    //     const Array<float>& kk){ return (rodrigues1(kk(0), kk(1), kk(2)), x.row_range(0, 3)); },
    //     kep.row_range(0, 3),
    //     float(1e-3));
    FixedArray<float, 3, 3> da_dkep = tait_bryan_angles_dtheta(
        kep.template row_range<0, 3>(),
        x.template row_range<0, 3>());

    auto da_dk = FixedArray<float, 3, 6>::init(
        da_dkep(0, 0), da_dkep(0, 1), da_dkep(0, 2), 1.f, 0.f, 0.f,
        da_dkep(1, 0), da_dkep(1, 1), da_dkep(1, 2), 0.f, 1.f, 0.f,
        da_dkep(2, 0), da_dkep(2, 1), da_dkep(2, 2), 0.f, 0.f, 1.f);
    return dot2d(dy_da, da_dk);
}

FixedArray<float, 2, 4> Mlib::Cv::projected_points_jacobian_dki_1p_1ke(
    const FixedArray<float, 3>& x,
    const TransformationMatrix<float, float, 3>& ke)
{
    FixedArray<float, 3> p = ke.transform(x);
    // ki = kip(0), 0      kip(1)
    //      0       kip(2) kip(3)
    //      0,      0      1
    // y = ki * p / p(2) = | kip(0) * p(0) + kip(1) * p(2) | / p(2)
    //                     | kip(2) * p(1) + kip(3) * p(2) |
    //                     | p(2)                          |
    auto m_dy_dkip = FixedArray<float, 2, 4>::init(
        p(0), p(2), 0.f, 0.f,
        0.f, 0.f, p(1), p(2));
    return m_dy_dkip / p(2);
}

/*
 * Only works with normalized coordinates
 */
FixedArray<float, 3> Mlib::Cv::reconstructed_point_(
    const Array<FixedArray<float, 2>>& y_tracked,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<TransformationMatrix<float, float, 3>>& ke,
    const Array<float>* weights,
    bool method2,
    bool points_are_normalized,
    float* condition_number,
    Array<float>* squared_distances,
    Array<FixedArray<float, 2>>* projection_residual)
{
    assert(y_tracked.ndim() == 1);
    assert(ke.ndim() == 1);
    assert(ke.length() == y_tracked.length());
    if (weights != nullptr) {
        assert(weights->length() == y_tracked.length());
    }
    //f ----a---- f+a
    //  ----x-f--  | f+a-x
    //            \x
    //vv^T(x-f)=a
    //f+a-x=f-vv^Tf+(vv^T-I)x
    //(vv^T-I)x=(vv^T-I)f
    //f = e.g. camera center
    Array<float> M;
    if (method2) {
        M = Array<float>(ArrayShape{y_tracked.length() * 3, y_tracked.length() + 3});
        M = 0;
    } else {
        M = Array<float>(ArrayShape{y_tracked.length() * 3, 3});
    }
    Array<float> B(ArrayShape{y_tracked.length() * 3});
    for (size_t r = 0; r < y_tracked.length(); ++r) {
        FixedArray<float, 3> v = uninitialized;
        FixedArray<double, 3> yl2 = homogenized_3(y_tracked(r)).casted<double>();
        if (points_are_normalized) {
            FixedArray<float, 3, 3> K3x3 = TransformationMatrix<float, float, 3>{ ki.project(ke(r).semi_affine()) }.R;
            v = lstsq_chol_1d(K3x3.casted<double>(), yl2).value().casted<float>(); //lstsq_chol_1d(K, y_tracked[r], float(1e-6));
        } else {
            FixedArray<float, 3, 3> K3x3 = ke(r).R;
            // Multiplying by "ki" first to scale the results into world-coordinates.
            v = lstsq_chol_1d(K3x3.casted<double>(), lstsq_chol_1d(ki.casted<double, double>().affine(), yl2).value()).value().casted<float>();
        }
        v /= std::sqrt(sum(squared(v)));
        //lerr() << "||v|| " << sum(squared(v));
        //lerr() << "yt " << y_tracked[r];
        //lerr() << "Kv " << (K3x3, v);

        //proj * f = [0; 0; 0; 1] => f = proj \ [0; 0; 0; 1]
        FixedArray<float, 3> f = ke(r).inverted().t;
        //lerr() << "hke\n" << homogenized_4x4(ke[r]);
        //lerr() << "Ki " << Ki;
        //lerr() << "K " << K;
        //lerr() << "f " << f;
        //lerr() << "ke f " << (ke[r], Array<float>{f(0), f(1), f(2), 1});

        if (method2) {
            for (size_t rr = 0; rr < 3; ++rr) {
                for (size_t c = 0; c < 3; ++c) {
                    M(rr + 3 * r, c) = (rr == c);
                }
                M(rr + 3 * r, 3 + r) = -v(rr);
            }
            B.row_range(3 * r, 3 * (r + 1)) = f;
        } else {
            const FixedArray<float, 3, 1>& v2 = v.reshaped<3, 1>();
            FixedArray<float, 3, 3> m = outer(v2, v2) - fixed_identity_array<float, 3>();
            FixedArray<float, 3> b = dot1d(m, f);
            if (weights != nullptr) {
                m *= (*weights)(r);
                b *= (*weights)(r);
            }
            M.row_range(3 * r, 3 * (r + 1)) = m;
            B.row_range(3 * r, 3 * (r + 1)) = b;
        }
    }
    //lerr() << "M " << M;
    //lerr() << "B " << B;
    if (condition_number != nullptr) {
        *condition_number = (float)cond4_x(M.casted<double>());
    }
    FixedArray<float, 3> x = uninitialized;
    if (method2) {
        if (squared_distances != nullptr) {
            throw std::runtime_error("Squared distances not supported for method2");
        }
        x = FixedArray<float, 3>{ lstsq_chol_1d(M, B).value().row_range(0, 3) };
    } else {
        x = FixedArray<float, 3>{ lstsq_chol_1d(
            M.casted<double>(),
            B.casted<double>()).value().casted<float>() };
        if (squared_distances != nullptr) {
            Array<float> diff = dot1d(M, x.to_array()) - B;
            *squared_distances = sum(squared(diff->reshaped(ArrayShape{y_tracked.length(), 3})), 1);
        }
    }
    if (projection_residual != nullptr) {
        *projection_residual = projected_points(
            Array<FixedArray<float, 3>>{ x },
            ki,
            ke).flattened() - y_tracked;
    }
    return x;
}

FixedArray<float, 3> Mlib::Cv::reconstructed_point_reweighted(
    const Array<FixedArray<float, 2>>& y_tracked,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<TransformationMatrix<float, float, 3>>& ke)
{
    Array<float> fs{ArrayShape{y_tracked.length(), 3}};
    FixedArray<float, 3> x = reconstructed_point_(y_tracked, ki, ke, nullptr, false);
    for (size_t i = 0; i < 1; ++i) {
        Array<float> weights{ArrayShape{y_tracked.length()}};
        for (size_t r = 0; r < weights.length(); ++r) {
            weights(r) =  1 / std::sqrt(sum(squared(x - ke(r).inverted().t)));
        }
        x = reconstructed_point_(y_tracked, ki, ke, &weights, false);
    }
    return x;
}
