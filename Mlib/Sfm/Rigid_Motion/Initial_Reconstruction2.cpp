#include "Initial_Reconstruction2.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Ransac.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

static Array<float> unpack_x(const Array<float>& k, size_t nx);
static Array<float> unpack_k_external(const Array<float>& k, size_t nintrinsic, size_t nx);

static Array<float> unpack_k_internal(const Array<float>& k) {
    assert(k.ndim() == 1);
    return k_internal(k.row_range(0, 4));
}

static Array<float> unpack_k_internal_dk(const Array<float>& k, const Array<float>& x, const Array<float>& ke) {
    assert(k.ndim() == 1);
    // t, i, y, k
    Array<float> result{ArrayShape{ke.shape(0), x.shape(0), 3, 4}};
    for (size_t t = 0; t < ke.shape(0); ++t) {
        for (size_t i = 0; i < x.shape(0); ++i) {
            result[t][i][2] = 0;
            result[t][i].row_range(0, 2) = projected_points_jacobian_dki_1p_1ke(x[i], k.row_range(0, 4), ke[t]);
        }
    }
    return result;
}

static Array<float> unpack_k_external(const Array<float>& k, size_t nintrinsic, size_t nx) {
    assert(k.ndim() == 1);
    assert((k.shape(0) - nintrinsic - nx) % 6 == 0);
    Array<float> result{ArrayShape{(k.shape(0) - nintrinsic - nx) / 6, 3, 4}};
    for (size_t i = nintrinsic; i < k.shape(0) - nx; i += 6) {
        result[(i - nintrinsic) / 6] = k_external(k.row_range(i, i + 6));
    }
    return result;
}

static Array<float> unpack_k_external_dk(const Array<float>& ki, const Array<float>& k, size_t nintrinsic, size_t nx, const Array<float>& x) {
    assert(k.ndim() == 1);
    assert((k.shape(0) - nintrinsic - nx) % 6 == 0);
    // t, i, y, k
    Array<float> result{ArrayShape{(k.shape(0) - nintrinsic - nx) / 6, x.shape(0), 3, 6}};
    for (size_t t = nintrinsic; t < k.shape(0) - nx; t += 6) {
        for (size_t i = 0; i < x.shape(0); ++i) {
            result[(t - nintrinsic) / 6][i][2] = 0;
            result[(t - nintrinsic) / 6][i].row_range(0, 2) = projected_points_jacobian_dke_1p_1ke(
                x[i],
                ki,
                k.row_range(t, t + 6));
        }
    }
    return result;
}

static Array<float> unpack_x(const Array<float>& k, size_t nx) {
    assert(nx > 0);
    assert(nx % 3 == 0);
    Array<float> result;
    result = k.row_range(k.shape(0) - nx, k.shape(0));
    result.do_reshape(ArrayShape{nx / 3, 3});
    return result;
}

static Array<float> unpack_x_dx(const Array<float>& ki, const Array<float>& x, const Array<float>& ke) {
    // t, i, y, x
    Array<float> result{ArrayShape{ke.shape(0), x.shape(0), 3, 3}};
    for (size_t t = 0; t < ke.shape(0); ++t) {
        for (size_t i = 0; i < x.shape(0); ++i) {
            result[t][i][2] = 0;
            result[t][i].row_range(0, 2) = projected_points_jacobian_dx_1p_1ke(x[i], ki, ke[t]).col_range(0, 3);
        }
    }
    return result;
}

/*
 * Only works with normalized coordinates
 */
Array<float> Mlib::Sfm::initial_reconstruction(
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& ki,
    const Array<float>& y0,
    const Array<float>& y1,
    bool points_are_normalized,
    Array<float>* condition_number)
{
    assert(all(y0.shape() == y1.shape()));
    assert(y0.ndim() == 2);
    assert(y0.shape(1) == 3);
    assert(all(R.shape() == ArrayShape{3, 3}));
    assert(all(t.shape() == ArrayShape{3}));
    if (condition_number != nullptr) {
        if (!condition_number->initialized()) {
            condition_number->do_resize(ArrayShape{y0.shape(0)});
        }
        assert(condition_number->length() == y0.shape(0));
    }

    Array<float> ke(std::list<Array<float>>({
        assemble_homogeneous_3x4(
            identity_array<float>(3),
            zeros<float>(ArrayShape{3})),
        assemble_inverse_homogeneous_3x4(R, t)}));
    Array<float> x(ArrayShape{y0.shape(0), 3});
    Array<float> y_tracked(ArrayShape{2, 3});
    for (size_t i = 0; i < y0.shape(0); ++i) {
        y_tracked[0] = y0[i];
        y_tracked[1] = y1[i];
        x[i] = reconstructed_point(y_tracked, ki, ke,
            nullptr, // weights
            nullptr, // fs
            false,   // method2
            points_are_normalized,
            condition_number == nullptr ? nullptr : &(*condition_number)(i));
    }
    return x;
}

/*
 * Reconstruct only the 3rd component (z) of x.
 * Did not get this working.
 * R.T seems to work better than R but it is then still not exact.
 * Using "initial_reconstruction" instead.
 */
Array<float> Mlib::Sfm::initial_reconstruction_x3(
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& ki,
    const Array<float>& y0,
    const Array<float>& y1,
    bool verbose)
{
    assert(all(y0.shape() == y1.shape()));
    assert(y0.ndim() == 2);
    assert(y0.shape(1) == 3);
    assert(all(R.shape() == ArrayShape{3, 3}));
    assert(all(t.shape() == ArrayShape{3}));

    //std::cerr << "t " << t << std::endl;
    //std::cerr << "R\n" << R << std::endl;
    //std::cerr << y1 << std::endl;
    //std::cerr << y1.shape() << std::endl;

    //x3: the third component of x, a.k.a. z
    Array<float> x3(y1.shape().erased_last().appended(2));
    if (verbose) {
        std::cerr << "R\n" << R << std::endl;
        std::cerr << "t " << t << std::endl;
        std::cerr << "y0 " << lstsq_chol_1d(ki, y0[0]) << std::endl;
        std::cerr << "y1 " << lstsq_chol_1d(ki, y1[0]) << std::endl;
    }
    y1.shape().erased_last().foreach([&](const ArrayShape& index){
        const Array<float> yy0 = lstsq_chol_1d(ki, y0[index]);
        const Array<float> yy1 = lstsq_chol_1d(ki, y1[index]);
        Array<float> c0 = R[0] - yy1(0) * R[2];
        Array<float> c1 = R[1] - yy1(1) * R[2];
        if (verbose) {
            std::cerr << "yy0 " << yy0 << std::endl;
            std::cerr << "yy1 " << yy1 << std::endl;
            std::cerr << "c0 " << c0 << std::endl;
            std::cerr << "c1 " << c1 << std::endl;
            std::cerr << "(c0, t) " << dot0d(c0, t) << std::endl;
            std::cerr << "(c0, yy0) " << dot0d(c0, yy0) << std::endl;
        }
        x3[index](0) = dot0d(c0, t) / dot0d(c0, yy0);
        x3[index](1) = dot0d(c1, t) / dot0d(c1, yy0);
    });
    if (verbose) {
        std::cerr << std::endl;
        std::cerr << "----------------" << std::endl;
        //std::cerr << "x3\n" << x3 << std::endl;
        //std::cerr << x3.shape() << std::endl;
        std::cerr << "R\n" << R << std::endl;
        std::cerr << "t " << t << std::endl;
        std::cerr << "ok (>0): " << all(x3 > 0.f) << std::endl;
        //std::cerr << "ok: " << min(x3) << std::endl;
        //The check should be about "> 0", but this was not enough.
        std::cerr << "min(x3) " << min(x3) << std::endl;
    }
    return x3;
}

void Mlib::Sfm::find_projection_matrices(
    const Array<float>& x,
    const Array<float>& y,
    const Array<float>* ki_precomputed,
    const Array<float>* kep_initial,
    Array<float>* ki_out,
    Array<float>* ke_out,
    Array<float>* kep_out,
    Array<float>* x_out,
    float alpha,
    float beta,
    float alpha2,
    float beta2,
    float min_redux,
    size_t niterations,
    size_t nburnin,
    size_t nmisses,
    bool print_residual,
    bool nothrow,
    Array<float>* final_residual,
    const float* max_residual,
    bool differentiate_numerically)
{
    assert(x.ndim() == 2);
    assert(y.ndim() == 3);
    assert(x.shape(0) == y.shape(1));
    assert(x.shape(1) == 4);
    assert(y.shape(2) == 3);
    size_t nintrinsic = ki_precomputed != nullptr ? 0 : 4;
    size_t nx = x_out != nullptr ? x.shape(0) * 3 : 0;
    Array<float> k0 = zeros<float>(ArrayShape{nintrinsic + y.shape(0) * 6 + nx});
    if (ki_precomputed == nullptr) {
        k0(0) = 1;
        k0(1) = 0;
        k0(2) = 1;
        k0(3) = 0;
    }
    if (kep_initial != nullptr) {
        assert(kep_initial->length() == y.shape(0) * 6);
        k0.row_range(
            nintrinsic,
            nintrinsic + y.shape(0) * 6) = *kep_initial;
    }
    if (x_out != nullptr) {
        k0.row_range(
            k0.length() - nx,
            k0.length()) = dehomogenized_Nx3(x).flattened();
    }
    // NormalizedProjection(np.pn).print_min_max();
    auto f = [&](const Array<float>& k){
        auto ki = ki_precomputed != nullptr ? *ki_precomputed : unpack_k_internal(k);
        auto ke = unpack_k_external(k, nintrinsic, nx);
        auto xx = x_out != nullptr
            ? homogenized_Nx4(unpack_x(k, nx))
            : x;
        auto s = projected_points(xx, ki, ke);
        return s.flattened();
    };
    Array<float> k_opt = levenberg_marquardt(
        k0,
        y.flattened(),
        f,
        [&](const Array<float>& k){
            if (differentiate_numerically) {
                return numerical_differentiation(f, k, float(1e-4));
            } else {
                Array<float> ki = ki_precomputed != nullptr ? *ki_precomputed : unpack_k_internal(k);
                Array<float> ke = unpack_k_external(k, nintrinsic, nx);
                Array<float> xx = x_out == nullptr ? x : homogenized_Nx4(unpack_x(k, nx));

                Array<float> J = zeros<float>(ArrayShape{y.shape(0), y.shape(1), 3, k.length()});
                Array<float> dy_dki;
                if (ki_precomputed == nullptr) {
                    dy_dki = unpack_k_internal_dk(k, xx, ke);
                }
                Array<float> dy_dke = unpack_k_external_dk(
                    ki,
                    k,
                    nintrinsic,
                    nx,
                    xx);
                Array<float> dy_dx;
                if (x_out != nullptr) {
                    dy_dx = unpack_x_dx(ki, xx, ke);
                }
                for (size_t t = 0; t < y.shape(0); ++t) {
                    for (size_t i = 0; i < y.shape(1); ++i) {
                        for (size_t d = 0; d < 3; ++d) {
                            if (ki_precomputed == nullptr) {
                                for (size_t j = 0; j < 4; ++j) {
                                    J(t, i, d, j) = dy_dki(t, i, d, j);
                                }
                            }
                            for (size_t j = 0; j < 6; ++j) {
                                J(t, i, d, nintrinsic + t * 6 + j) = dy_dke(t, i, d, j);
                            }
                            if (x_out != nullptr) {
                                for (size_t j = 0; j < 3; ++j) {
                                    J(t, i, d, nintrinsic + y.shape(0) * 6 + i * 3 + j) = dy_dx(t, i, d, j);
                                }
                            }
                        }
                    }
                }
                // auto an = J.rows_as_1D();
                // auto nu = numerical_differentiation(f, k, float(1e-4));
                // an.save_txt_2d("an.m");
                // nu.save_txt_2d("nu.m");
                // assert(false);
                // return nu;
                return J.rows_as_1D();
            }
        },
        alpha,
        beta,
        alpha2,
        beta2,
        min_redux,
        niterations,
        nburnin,
        nmisses,
        print_residual,
        nothrow,
        final_residual,
        max_residual);
    if (ki_out != nullptr) {
        *ki_out = ki_precomputed != nullptr ? *ki_precomputed : unpack_k_internal(k_opt);
    }
    if (ke_out != nullptr) {
        *ke_out = unpack_k_external(k_opt, nintrinsic, nx);
    }
    if (kep_out != nullptr) {
        *kep_out = k_opt.row_range(nintrinsic, nintrinsic + y.shape(0) * 6);
    }
    if (x_out != nullptr) {
        *x_out = homogenized_Nx4(unpack_x(k_opt, nx));
    }
    if (final_residual != nullptr) {
        assert(final_residual->length() == y.nelements());
        final_residual->do_reshape(y.shape());
    }
}

void Mlib::Sfm::find_projection_matrices_ransac(
    const RansacOptions<float>& ro,
    const Array<float>& x,
    const Array<float>& y,
    const Array<float>* ki_precomputed,
    Array<float>* ki_out,
    Array<float>* ke_out,
    Array<float>* kep_out,
    Array<float>* x_out,
    float alpha,
    float beta,
    float alpha2,
    float beta2,
    float min_redux,
    size_t niterations,
    size_t nburnin,
    size_t nmisses,
    bool print_residual,
    bool nothrow,
    Array<float>* final_residual)
{
    auto gen_y1 = [&](const Array<size_t>& indices) {
        Array<float> y1{ArrayShape{y.shape(0), indices.length(), y.shape(2)}};
        for (size_t i = 0; i < y.shape(0); ++i) {
            y1[i] = y[i][indices];
        }
        return y1;
    };

    const Array<size_t> best_indices = ransac(
        y.shape(1), // nelems_large
        ro,
        [&](const Array<size_t>& indices) {
            Array<float> final_residual1;
            Array<float> ki_out1;
            Array<float> ke_out1;
            find_projection_matrices(
                x[indices],
                gen_y1(indices),
                ki_precomputed,
                nullptr,           // kep_initial
                &ki_out1,
                &ke_out1,
                nullptr,           // kep_out
                nullptr,           // x_out
                alpha,
                beta,
                alpha2,
                beta2,
                min_redux,
                niterations,
                nburnin,
                nmisses,
                print_residual,
                nothrow,
                &final_residual1);
            Array<float> residual = projected_points(x, ki_out1, ke_out1) - y;
            Array<float> residual2 = zeros<float>(ArrayShape{y.shape(1)});
            for (size_t i = 0; i < residual.shape(0); ++i) {
                for (size_t j = 0; j < residual.shape(1); ++j) {
                    for (size_t k = 0; k < residual.shape(2); ++k) {
                        residual2(j) += squared(residual(i, j, k));
                    }
                }
            }
            return residual2;
        }
    );
    if (!best_indices.initialized()) {
        throw std::runtime_error("RANSAC could not find a good projection");
    }
    find_projection_matrices(
        x[best_indices],
        gen_y1(best_indices),
        ki_precomputed,
        nullptr,               //kep_initial
        ki_out,
        ke_out,
        kep_out,
        x_out,
        alpha,
        beta,
        alpha2,
        beta2,
        min_redux,
        niterations,
        nburnin,
        nmisses,
        print_residual,
        nothrow,
        final_residual);
}

void Mlib::Sfm::find_projection_matrices_twopass(
    const Array<float>& x,
    const Array<float>& y,
    const Array<float>* ki_precomputed,
    Array<float>* ki_out,
    Array<float>* ke_out,
    Array<float>* kep_out,
    Array<float>* x_out,
    float alpha,
    float beta,
    float alpha2,
    float beta2,
    float min_redux,
    size_t niterations,
    size_t nburnin,
    size_t nmisses,
    bool print_residual,
    bool nothrow,
    Array<float>* final_residual,
    const float* max_residual)
{
    Array<float> kep_out1;
    find_projection_matrices(
        x,                 // x
        y,                 // y
        ki_precomputed,    // ki_precomputed
        nullptr,           // kep_initial
        nullptr,           // ki_out
        nullptr,           // ke_out
        &kep_out1,         // kep_out
        nullptr,           // x_out
        alpha,             // alpha
        beta,              // beta
        alpha2,            // alpha2
        beta2,             // beta2
        min_redux,         // min_redux
        niterations,       // niterations
        nburnin,           // nburnin
        nmisses,           // nmisses
        print_residual,    // print_residual
        nothrow,           // nothrow
        nullptr,           // final_residual
        max_residual);     // max_residual

    find_projection_matrices(
        x,                 // x
        y,                 // y
        ki_precomputed,    // ki_precomputed
        &kep_out1,         // kep_initial
        ki_out,            // ki_out
        ke_out,            // ke_out
        kep_out,           // kep_out
        x_out,             // x_out
        alpha,             // alpha
        beta,              // beta
        alpha2,            // alpha2
        beta2,             // beta2
        min_redux,         // min_redux
        niterations,       // niterations
        nburnin,           // nburnin
        nmisses,           // nmisses
        print_residual,    // print_residual
        nothrow,           // nothrow
        final_residual,    // final_residual
        max_residual);     // max_residual
}

Array<float> Mlib::Sfm::find_epipole(
    const Array<float>& ki,
    const Array<float>& ke)
{
    Array<float> t = t3_from_Nx4(ke, 3);
    Array<float> R = R3_from_Nx4(ke, 3);
    Array<float> ti;
    Array<float> Ri;
    invert_t_R(t, R, ti, Ri);
    return projected_points_1p_1ke(
        homogenized_4(ti),
        ki,
        identity_array<float>(4).row_range(0, 3),
        true);  // allow_points_at_fininity
}
