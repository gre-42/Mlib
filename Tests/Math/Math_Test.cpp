#include <Mlib/Math/Approximate_Rank.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Gaussian_Elimination.hpp>
#include <Mlib/Math/Huber_Norm.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Non_Zero_Ids.hpp>
#include <Mlib/Math/Optimize/Cg.hpp>
#include <Mlib/Math/Set_Difference.hpp>
#include <Mlib/Math/Sort_Svd.hpp>
#include <Mlib/Math/Svd4.hpp>
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;

void test_svd() {
    Array<float> a = random_array<float>(ArrayShape{3, 2});
    Array<float> uT;
    Array<float> s;
    Array<float> vT;
    svd(a, uT, s, vT);
    /*std::cerr.setf(std::ios_base::scientific);
    std::cerr << "a" << std::endl;
    std::cerr << a << std::endl;
    std::cerr << "u" << std::endl;
    std::cerr << uT.T() << std::endl;
    std::cerr << "s" << std::endl;
    std::cerr << s << std::endl;
    std::cerr << "v" << std::endl;
    std::cerr << v << std::endl;*/
    assert_isclose<float>(a(2, 1), 407);
    assert_allclose<float>(uT.T(), Array<float>{{3.225679e-02, 1.277676e-01},
                                                {6.286538e-01, 7.684482e-01},
                                                {7.770160e-01, -6.270269e-01}});
    assert_allclose<float>(s, Array<float>{9.966705e+02, 2.68333649e+02});
    assert_allclose<float>(vT, Array<float>{{7.294542e-01, 6.840296e-01},
                                            {-6.840298e-01, 7.294541e-01}});

    Array<float> ui;
    float si;
    inverse_iteration_symm(dot(a.T(), a), ui, si);
    assert_allclose<float>(ui, Array<float>{-6.840296e-01, 7.294543e-01});
    assert_isclose<float>(std::sqrt(si), 268.333618, 1e-4);
}

void test_qdq() {
    Array<float> a = random_array<float>(ArrayShape{3, 3}, 1);
    Array<float> q;
    Array<float> s;
    a = -outer(a, a);
    qdq(a, q, s);
    /*std::cerr.setf(std::ios_base::scientific);
    std::cerr << "a" << std::endl;
    std::cerr << a << std::endl;
    std::cerr << "q" << std::endl;
    std::cerr << q << std::endl;
    std::cerr << "s" << std::endl;
    std::cerr << s << std::endl;*/
    assert_isclose<float>(q(1, 0) / sign(q(0, 0)), -0.149676);
    assert_isclose<float>(s(0), 447580.468750);
    assert_isclose<float>(s(1), 64099.5);
}

void test_det_2x2() {
    Array<float> a = random_array2<float>(ArrayShape{2, 2}, 1);
    Array<float> q;
    Array<float> s;
    a = outer(a, a) + identity_array<float>(2) * 0.01f;
    qdq(a, q, s);
    assert_isclose(det2x2(a), s(0) * s(1));
}

void test_det_3x3() {
    Array<float> a({
        {6, 1, 1},
        {4, -2, 5},
        {2, 8, 7}});
    assert_isclose<float>(det3x3(a), -306);
}

void test_identity() {
    Array<float> a;
    a.resize[5](4);
    identity_array(a);
    assert(a(0, 0) == 1);
    assert(a(1, 1) == 1);
    assert(a(1, 0) == 0);
}

void test_lstsq() {
    Array<float> a = random_array<float>(ArrayShape{5, 3}, 1);
    Array<float> b = random_array<float>(ArrayShape{5, 2}, 2);
    assert_allclose(dot(pinv(a), a), identity_array<float>(3), float(1e-4));
    assert_allclose(pinv(a), pinv(a.T()).T());
    assert_allclose(lstsq(a, a), identity_array<float>(3), float(1e-4));
    assert_allclose(lstsq(a, 1.4f * a), 1.4f * identity_array<float>(3), float(1e-4));
}

void test_lstsq_complex() {
    Array<std::complex<float>> ar;
    Array<std::complex<float>> ai;
    ar.resize[5](3);
    ai.resize[5](3);
    randomize_array(ar, 1);
    randomize_array(ai, 2);
    Array<std::complex<float>> a = ar + std::complex<float>(0, 1) * ai;
    assert_allclose(dot(pinv(a), a), identity_array<std::complex<float>>(3), float(1e-3));
    assert_allclose(lstsq(a, a), identity_array<std::complex<float>>(3), float(1e-3));
}

void test_lstsq_chol() {
    Array<float> a = random_array<float>(ArrayShape{5, 3}, 1);
    assert_allclose(lstsq_chol(a, a), identity_array<float>(3));
}

void test_lstsq_chol_complex() {
    auto a = random_complex_array<float>(ArrayShape{5, 3}, 1);
    assert_allclose(lstsq_chol(a, a), identity_array<std::complex<float>>(3));

    auto as = random_complex_array<float>(ArrayShape{5, 5}, 1);
    auto b = random_complex_array<float>(ArrayShape{5, 2}, 3);
    assert_allclose(dot(as, lstsq_chol(as, b)), b, 1e-2);
}

void test_sort_svd() {
    Array<float> u{{1, 2, 3}, {4, 5, 6}};
    Array<float> s{3, 1, 2};
    Array<float> vT{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    sort_svd(u, s, vT);
    assert_allclose(u, Array<float>{{1, 3, 2}, {4, 6, 5}});
    assert_allclose(s, Array<float>{3, 2, 1});
    assert_allclose(vT, Array<float>{{1, 2, 3}, {7, 8, 9}, {4, 5, 6}});
}

void test_outer() {
    Array<float> a = random_array<float>(ArrayShape{3, 4});
    Array<float> b2 = random_array<float>(ArrayShape{1, 4});
    Array<float> b1 = random_array<float>(ArrayShape{4});
    assert_allclose(outer(a, b2).flattened(), dot(a, b1));
    assert_isclose(outer(b2, b2)(0, 0), dot0d(b1, b1));
}

void test_solve_svd() {
    Array<float> a = random_array2<float>(ArrayShape{5, 3}, 1);
    Array<float> uT;
    Array<float> s;
    Array<float> vT;
    svd(a, uT, s, vT);
    assert_allclose(reconstruct_svd(uT.T(), s, vT), a, 1e-3);
    assert_allclose(dot(pinv_svd(uT.T(), s, vT), a), identity_array<float>(3), 1e-3);
}

void test_solve_svd_complex() {
    Array<std::complex<float>> a = (
        random_array4<std::complex<float>>(ArrayShape{5, 3}, 1) +
        std::complex<float>(0, 1) *
        random_array4<std::complex<float>>(ArrayShape{5, 3}, 2));
    Array<std::complex<float>> uT;
    Array<float> s;
    Array<std::complex<float>> vT;
    svd(a, uT, s, vT);
    assert_allclose(reconstruct_svd(uT.H(), s, vT), a, 1e-3);
    assert_allclose(dot(pinv_svd(uT.H(), s, vT), a), identity_array<std::complex<float>>(3), 1e-3);
}

void test_cond() {
    Array<double> a = random_array2<double>(ArrayShape{5, 3}, 1);
    assert_isclose(cond(a), 29.612, 1e-3);
    assert_isclose(cond4(a), 29.612, 1e-3);
}

void test_approximate_rank() {
    Array<double> a = dot(
        random_array4<double>(ArrayShape{6, 2}, 1),
        random_array4<double>(ArrayShape{2, 5}, 2));
    assert_allclose(approximate_rank(a, 2), a);
    assert_isclose(approximate_rank(a, 1)(0, 0), 0.728259);
}

void test_batch_dot() {
    Array<float> a = random_array4<float>(ArrayShape{3, 4}, 1);
    Array<float> b = random_array4<float>(ArrayShape{4, 5}, 2);
    assert_allclose(dot(a, b), batch_dot(a, b));
}

void test_huber_norm() {
    Array<double> x = linspace<double>(0, 5, 10);
    assert_allclose(
        huber_norm(x, 3., false),
        Array<double>{
            0, 0.0514403, 0.205761, 0.462963, 0.823045,
            1.28601, 1.83333, 2.38889, 2.94444, 3.5},
        1e-5);

    Array<double> x2 = Array<double>({x / std::sqrt(2), x / std::sqrt(2)});
    assert_allclose(
        huber_norm(x2, 3., true),
        Array<double>{
            0, 0.0514403, 0.205761, 0.462963, 0.823045,
            1.28601, 1.83333, 2.38889, 2.94444, 3.5},
        1e-5);
}

void test_sum_axis() {
    Array<float> a{0, 1, 2};
    Array<float> b({a, 4.f * a});
    assert_isclose<float>(sum(a, 0)(), 3);
    assert_allclose<float>(sum(b, 0), Array<float>{0, 5, 10});
    assert_allclose<float>(sum(b, 1), Array<float>{3, 12});

    Array<float> c = random_array4<float>(ArrayShape{2, 4, 5}, 1);
    assert_allclose<float>(sum(c, 0), c[0] + c[1]);
}

void test_array_array_comparison() {
    assert_allequal(
        Array<float>{1, 3, 2} < Array<float>{2, 1, 4},
        Array<bool>{true, false, true});
    assert_allequal(
        FixedArray<float, 3>{1, 3, 2} < FixedArray<float, 3>{2, 1, 4},
        FixedArray<bool, 3>{true, false, true});
}

void test_interpolate() {
    assert_allclose(
        interpolate(
            Array<float>{0, 1, 2},
            Array<float>{10, 10.4, 10.6}),
        Array<float>{10, 10.4, 10.6});
    assert_allclose(
        interpolate(
            Array<float>{-0.1, 0.5, 2.1},
            Array<float>{10, 10.4, 10.6}),
        Array<float>{NAN, 10.2, NAN});
}

void test_substitute_nans() {
    Array<float> a{0, 1, 2, NAN, 4, 5, 6};
    assert_allclose(
        substitute_nans(a, -1.f),
        Array<float>{0, 1, 2, -1, 4, 5, 6});
}

void test_power() {
    assert_allclose(
        pow(10.f, Array<float>{0, 1, 2, NAN, 4, 5, 6}),
        Array<float>{1, 10, 100, NAN, 10000, 100000, 1e+6});
    assert_allclose(
        pow(Array<float>{0, 1, 2, NAN, 4, 5, 6}, 10.f),
        Array<float>{0, 1, 1024, NAN, 1.04858e+06, 9.76562e+06, 6.04662e+07},
        1e2);
}

void test_fixed_shape() {
    Array<float> ad = random_array4<float>(ArrayShape{3, 4}, 1);
    FixedArray<float, 3, 4> a{ad};
    // auto x = rm_last(a);
    FasUtils::reshape_fixed(a, FasUtils::make_shape(a).erased_first().concatenated(FixedArrayShape<3>()));
    FasUtils::make_shape(a).erased_last();
    FasUtils::rows_as_1D(FasUtils::make_shape(a));
    FasUtils::columns_as_1D(FasUtils::make_shape(a));

    Array<float> bd = random_array4<float>(ArrayShape{4, 5, 6}, 2);
    FixedArray<float, 4, 5, 6> b{bd};
    assert_isequal(dot(a, b).ndim(), dot(ad, bd).ndim());
    assert_isclose(
        dot(a, b)(1, 2, 3),
        dot(ad, bd)(1, 2, 3));
}

void test_fixed_outer() {
    Array<float> ad = random_array4<float>(ArrayShape{3, 4}, 1);
    Array<float> bd = random_array4<float>(ArrayShape{6, 5, 4}, 2);
    FixedArray<float, 3, 4> a{ad};
    FixedArray<float, 6, 5, 4> b{bd};

    assert_isequal(outer(a, b).ndim(), outer(ad, bd).ndim());
    assert_isclose(
        outer(a, b)(1, 2, 3),
        outer(ad, bd)(1, 2, 3));
}

void test_regularization() {
    auto Jg = random_array4<float>(ArrayShape{6, 3}, 1);
    auto residual = random_array4<float>(ArrayShape{6}, 2);
    auto x0 = random_array4<float>(ArrayShape{3}, 3);
    auto ATA = dot2d(Jg.vH(), Jg);
    auto ATr = dot1d(Jg.vH(), residual);

    Array<float> v1 = x0 + solve_symm_1d(ATA, ATr, float(1e-1), float(1e-1));
    Array<float> v2 = solve_symm_1d(ATA, ATr + dot1d(ATA, x0), float(1e-1), float(1e-1), &x0);

    assert_allclose(v1, v2);

    // return x + lstsq_chol_1d(Jg, residual, float(1e-2), float(1e-2));
}

void test_gaussian_elimination() {
    Array<float> a = random_array4<float>(ArrayShape{5, 5}, 2);
    Array<float> b = random_array4<float>(ArrayShape{5}, 2);
    Array<float> x = gaussian_elimination_1d(a, b);
    assert_allclose(dot(a, x), b);

    Array<float> ata = dot2d(a.T(), a);
    Array<float> c = solve_symm_1d(ata, b, 0.1f, 0.2f);
    Array<float> d = gaussian_elimination_1d(ata, b, 0.1f, 0.2f);
    assert_allclose(c, d);
}

void test_cg() {
    Array<float> a = random_array4<float>(ArrayShape{5, 5}, 1);
    Array<float> A = dot2d(a.vH(), a);
    Array<float> b = random_array4<float>(ArrayShape{A.shape(0)}, 2);
    Array<float> x = solve_symm_1d(A, b);
    Array<float> x0 = x + 10.f * random_array4<float>(x.shape(), 3);
    Array<float> x1 = cg_simple(A, x0, b, 100, float(1e-6));
    assert_allclose(x, x1, 1e-2);
}

void test_set_difference() {
    Array<size_t> a{20, 30, 1, 2, 3, 4, 5};
    Array<size_t> b{2, 3, 4, 5, 6, 7};
    assert_allequal(set_difference(a, b), Array<size_t>{1, 20, 30});
}

void test_nonzero_ids() {
    Array<size_t> a{
        {0, 3, 2, 3},
        {2, 2, 0, 5}};
    assert_allequal(
        nonzero_ids(a),
        Array<size_t>{
            {0, 1},
            {0, 2},
            {0, 3},
            {1, 0},
            {1, 1},
            {1, 3}});
}

void test_fixed_transposed() {
    FixedArray<float, 3, 4> a{random_array4<float>(ArrayShape{3, 4}, 1)};
    assert_allclose(a.to_array().T(), a.T().to_array());
}

void test_fixed_cholesky() {
    Array<float> a_ = random_array4<float>(ArrayShape{5, 5}, 1);
    Array<float> b = random_array4<float>(ArrayShape{5}, 2);
    Array<float> a = dot2d(a_.vH(), a_);
    FixedArray<float, 5, 5> fa{a};
    FixedArray<float, 5> fb{b};
    assert_allclose(
        solve_symm_1d(a, b),
        solve_symm_1d(fa, fb).to_array());
}

void test_interp() {
    Interp<float> interp{{0, 0.1, 2, 3}, {4.123, 2.567, 3.89, 4.2}, false, -10, 20};
    assert_allclose(
        Array<float>{interp(-1), interp(4.3)},
        Array<float>{-10, 20});
    assert_allclose(
        Array<float>{interp(0), interp(0.05), interp(0.1), interp(2), interp(3)},
        Array<float>{4.123, 3.345, 2.567, 3.89, 4.2});
}

int main(int argc, const char** argv) {
    test_svd();
    test_qdq();
    test_det_2x2();
    test_det_3x3();
    test_identity();
    test_lstsq();
    test_lstsq_complex();
    test_lstsq_chol();
    test_lstsq_chol_complex();
    test_sort_svd();
    test_outer();
    test_solve_svd();
    test_solve_svd_complex();
    test_cond();
    test_approximate_rank();
    test_batch_dot();
    test_huber_norm();
    test_sum_axis();
    test_array_array_comparison();
    test_interpolate();
    test_substitute_nans();
    test_power();
    test_fixed_shape();
    test_fixed_outer();
    test_regularization();
    test_gaussian_elimination();
    test_cg();
    test_set_difference();
    test_nonzero_ids();
    test_fixed_transposed();
    test_fixed_cholesky();
    test_interp();
    return 0;
}
