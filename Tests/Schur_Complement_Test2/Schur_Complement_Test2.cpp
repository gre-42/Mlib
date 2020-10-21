#include <Mlib/Math/Debug/Schur_Complement_Debug.hpp>
#include <Mlib/Math/Debug/Schur_Complement_Debug2.hpp>
#include <Mlib/Math/Schur_Complement.hpp>
#include <Mlib/Sfm/Marginalization/Marginalizing_Bias.hpp>
#include <Mlib/Sfm/Marginalization/Regrid_Array.hpp>
#include <Mlib/Sfm/Marginalization/Synthetic_Scene.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <chrono>
#include <set>

using namespace Mlib;
using namespace Mlib::Sfm::SynthMarg;

void test_schur_complement2() {
    Array<float> A{
        {1,        0,       0,    0},
        {1.1,   -1.1,       0,    0},
        {1.05, -1.05,       0,    0},
        {1.2,   -1.2,       0,    0},
        {0,        1,      -2,    0},
        {0,      1.1,    -2.1,    0},
        {0,     1.05,      -2,    0},
        {0,        0,       3,   -2},
        {0,        0,     3.1, -2.1},
        {0,        0,    3.05,  -2}};
    Array<float> x0{2, 3, 4, 5};
    Array<float> y{
        5, -0.1, 0.3, 0.2, -0.15, 0.25, 0.2, 0.1, -0.15, 0.05};
    SparseArrayCcs<float> J{A};
    Array<float> residual = y - dot1d(A, x0);
    Array<float> dx = lstsq_chol_1d(J, residual);
    assert_allclose(x0 + dx, Array<float>{5.00308, 4.88531, 2.4728, 3.70942}, 1e-5);
    x0(0) = (x0 + dx)(0);  // This means that x(0) can be marginalized without error.
    residual.reassign(y - dot1d(A, x0));
    {
        Array<float> dx = lstsq_chol_1d(J, residual);
        assert_allclose(x0 + dx, Array<float>{5.00308, 4.88531, 2.4728, 3.70942}, 1e-5);
    }

    float alpha = 1e-2;
    float beta = 1e-2;
    Array<size_t> ids_k{ArrayShape{0}};
    Array<size_t> ids_a{1, 2, 3};
    Array<size_t> ids_b{0};
    Array<size_t> ids_ka = ids_k.appended(ids_a);
    {
        Array<float> lhs_ka;
        Array<float> rhs_ka;
        //::Mlib::Debug::schur_complement_jacobian_system(J, residual, x0, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, ids_ka, alpha, beta);
        ::Mlib::Debug2::schur_complement_jacobian_system(J, residual, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, ids_ka, alpha, beta);
        //Array<float> dx = solve_symm_1d(lhs_ka.unblocked(ids_ka, ids_ka, ArrayShape{J.shape(1), J.shape(1)}, 0.f), rhs_ka.unblocked(ids_ka, J.shape(1), 0.f), alpha, beta);
        Array<float> dx = solve_symm_1d(lhs_ka, rhs_ka, alpha, beta);
        assert_isclose(sum(abs(lhs_ka - lhs_ka.T())), 0.f);
        //std::cerr << dx + dx.unblocked(ids_ka, x0.length()) << std::endl;
    }
    {
        Array<float> lhs_ka;
        Array<float> rhs_ka;
        marginalize_least_squares(J, residual, x0, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, alpha*0, beta*0);
        // std::cerr << dot2d(J.vH(), J) << std::endl;
        // std::cerr << lhs_ka.blocked(ids_ka, ids_ka) << std::endl;
        Array<float> dx = solve_symm_1d(lhs_ka.blocked(ids_ka, ids_ka), rhs_ka.blocked(ids_ka), alpha, beta);
        //std::cerr << x0 + dx.unblocked(ids_ka, x0.length()) << std::endl;
    }
    {
        Array<float> A = dot2d(J.vH(), J);
        Array<float> b = dot1d(J.vH(), residual);
        {
            Array<float> dx = solve_symm_1d(A, b);
            assert_allclose(x0 + dx, Array<float>{5.00308, 4.88531, 2.4728, 3.70942}, 1e-5);
        }
        {
            Array<float> dx = solve_symm_1d(A, b, alpha, beta);
            assert_allclose(x0 + dx, Array<float>{5.20492, 5.14403, 2.66678, 4.01099}, 1e-5);
        }
        {
            Array<float> lhs;
            Array<float> rhs;
            schur_complement_system(A, b, ids_a, ids_b, lhs, rhs, alpha, beta);
            Array<float> dx = solve_symm_1d(lhs, rhs, alpha, beta);
            assert_allclose(x0 + dx.unblocked(ids_a, x0.length()), Array<float>{NAN, 5.20331, 2.69573, 4.05394}, 1e-5);
        }
        {
            Array<float> lhs;
            Array<float> rhs;
            schur_complement_system(A, b, ids_a, ids_b, lhs, rhs, 0, 0);
            Array<float> dx = solve_symm_1d(lhs, rhs, alpha, beta);
            assert_allclose(x0 + dx.unblocked(ids_a, x0.length()), Array<float>{NAN, 5.2151, 2.70148, 4.06247}, 1e-5);
        }
    }
    {
        MarginalizingBias dsolver{alpha*0, beta*0};
        dsolver.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}});
        if (false) {
            Array<float> dx = dsolver.solve(J, x0, residual);
            assert_allclose(x0 + dx, Array<float>{5.00308, 4.88531, 2.4728, 3.70942}, 1e-5);
        }
        {
            Array<float> dx = solve_symm_1d(dot2d(J.vH(), J), dot1d(J.vH(), residual));
            assert_allclose(x0 + dx, Array<float>{5.00308, 4.88531, 2.4728, 3.70942}, 1e-5);
        }
        // dsolver.marginalize(J, x0, ids_a, ids_b);
        // if (false) {
        //     std::cerr << "rhs_ka_ " << dsolver.rhs_ka_ << std::endl;
        //     dsolver.update_indices({{UUID{1}, 0}, {UUID{2}, 1}, {UUID{3}, 2}});
        //     std::cerr << "rhs_ka_ " << dsolver.rhs_ka_ << std::endl;
        //     Array<float> dx = dsolver.solve(J.columns(ids_ka), x0, residual);
        //     std::cerr << "lhs_ka_" << std::endl;
        //     std::cerr << dsolver.lhs_ka_ << std::endl;
        //     std::cerr << "rhs_ka_" << std::endl;
        //     std::cerr << dsolver.rhs_ka_ << std::endl;
        //     std::cerr << x0 + dx.unblocked(ids_ka, x0.length()) << std::endl;
        // }
    }
}

void test_schur_complement3() {
    float alpha = 1e-2;
    float beta = 1e-2;

    Array<float> G = uniform_random_array<float>(ArrayShape{10, 5}, 1);
    SparseArrayCcs<float> J{G};
    Array<float> A = dot2d(G.vH(), G);
    Array<float> y = uniform_random_array<float>(ArrayShape{10}, 2);
    Array<float> b = dot(G.T(), y);

    Array<float> x_exact = solve_symm_1d(A, b);
    Array<float> x0 = x_exact + Array<float>{0.1, 0, 0, 0, 0};  // This means that all except x(0) can be marginalized without error.
    Array<float> residual = y - dot1d(G, x0);
    Array<float> b1 = dot(G.T(), residual);

    {
        Array<float> x = solve_symm_1d(A, b);
        assert_allclose(x, Array<float>{0.458309, 0.852957, -0.372384, 0.0442832, -0.291572}, 1e-5);
    }
    {
        Array<float> x = solve_symm_1d(A, b, alpha, beta);
        assert_allclose(x, Array<float>{0.444589, 0.786624, -0.322209, 0.039023, -0.237743}, 1e-5);
    }
    Array<size_t> ids_k{ArrayShape{0}};
    Array<size_t> ids_a{0, 1, 4};
    Array<size_t> ids_b{2, 3};
    Array<size_t> ids_ka = ids_k.appended(ids_a);

    SparseArrayCcs<float> J1{J.columns(ids_ka)};
    Array<float> x01 = x0.blocked(ids_ka);
    Array<float> residual1 = y - dot1d(J1.to_dense_array(), x01);
    Array<float> x_exact1 = x_exact.blocked(ids_ka);

    Array<size_t> ids_k2{ArrayShape{0}};
    Array<size_t> ids_a2{0, 1};
    Array<size_t> ids_b2{2};
    Array<size_t> ids_ka2 = ids_k2.appended(ids_a2);

    SparseArrayCcs<float> J2{J1.columns(ids_ka2)};
    Array<float> x02 = x01.blocked(ids_ka2);
    Array<float> residual2 = y - dot1d(J2.to_dense_array(), x02);
    Array<float> x_exact2 = x_exact1.blocked(ids_ka2);

    Array<size_t> ids_k3{ArrayShape{0}};
    Array<size_t> ids_a3{0};
    Array<size_t> ids_b3{1};
    Array<size_t> ids_ka3 = ids_k3.appended(ids_a3);

    SparseArrayCcs<float> J3{J2.columns(ids_ka3)};
    Array<float> x03 = x02.blocked(ids_ka3);
    Array<float> residual3 = y - dot1d(J3.to_dense_array(), x03);
    Array<float> x_exact3 = x_exact2.blocked(ids_ka3);

    {
        Array<float> lhs;
        Array<float> rhs;
        schur_complement_system(A, b, ids_a, ids_b, lhs, rhs, 0, 0);
        Array<float> x = solve_symm_1d(lhs, rhs, 0.f, 0.f);
        assert_allclose(x.unblocked(ids_a, b.length()), Array<float>{0.458309, 0.852957, NAN, NAN, -0.291572}, 1e-5);
    }
    {
        Array<float> lhs;
        Array<float> rhs;
        schur_complement_system(A, b1, ids_a, ids_b, lhs, rhs, 0, 0);
        Array<float> dx = solve_symm_1d(lhs, rhs, 0.f, 0.f);
        assert_allclose(x0 + dx.unblocked(ids_a, b.length()), Array<float>{0.458309, 0.852957, NAN, NAN, -0.291572}, 1e-5);
    }
    {
        MarginalizingBias dsolver{alpha*0, beta*0};
        dsolver.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}, {UUID{4}, 4}});
        {
            Array<float> dx = dsolver.solve(J, x0, residual);
            Array<float> x = x0 + dx;
            assert_allclose(x, Array<float>{0.458309, 0.852957, -0.372384, 0.0442832, -0.291572}, 1e-5);
        }

        {
            // WIP
            MarginalizingBias dsolver_wip{alpha*0, beta*0};
            dsolver_wip.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}, {UUID{4}, 4}});
            dsolver_wip.marginalize_wip(J, x0, residual, ids_a, ids_b);
            dsolver_wip.update_indices({{UUID{ids_ka(0)}, 0}, {UUID{ids_ka(1)}, 1}, {UUID{ids_ka(2)}, 2}});
            Array<float> dx = dsolver_wip.solve(J1, x01, residual);
            Array<float> x1 = x01 + dx;
            assert_allclose(x1, Array<float>{0.458309, 0.852957, -0.291572});
        }
        {
            // Exact
            MarginalizingBias dsolver_exact{alpha*0, beta*0};
            dsolver_exact.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}, {UUID{4}, 4}});
            dsolver_exact.marginalize(J, x_exact, ids_a, ids_b);
            dsolver_exact.update_indices({{UUID{ids_ka(0)}, 0}, {UUID{ids_ka(1)}, 1}, {UUID{ids_ka(2)}, 2}});
            {
                Array<float> dx = dsolver_exact.solve(J1, x01, residual);
                Array<float> x1 = x01 + dx;
                assert_allclose(x1, Array<float>{0.458309, 0.852957, -0.291572});
            }
            {
                Array<float> dx = dsolver_exact.solve(J1, x01, residual);
                Array<float> x1 = x01 + dx;
                assert_allclose(x1, Array<float>{0.458309, 0.852957, -0.291572});
                assert_isclose(dsolver_exact.bias(x01), 1.71858f, 1e-5);
            }

            // Exact2
            dsolver_exact.marginalize(J1, x_exact1, ids_a2, ids_b2);
            dsolver_exact.update_indices({{UUID{ids_ka2(0)}, 0}, {UUID{ids_ka2(1)}, 1}});
            {
                Array<float> dx2 = dsolver_exact.solve(J2, x02, residual);
                Array<float> x2 = x02 + dx2;
                assert_allclose(x2, Array<float>{0.458309, 0.852957});
                assert_isclose(dsolver_exact.bias(x02), 2.64965f, 1e-5);
            }

            // Exact3
            dsolver_exact.marginalize(J2, x_exact2, ids_a3, ids_b3);
            dsolver_exact.update_indices({{UUID{ids_ka3(0)}, 0}});
            {
                Array<float> dx3 = dsolver_exact.solve(J3, x03, residual);
                Array<float> x3 = x03 + dx3;
                assert_allclose(x3, Array<float>{0.458309});
                assert_isclose(dsolver_exact.bias(x03), 0.222736f, 1e-5);
            }
        }
        {
            // Not exact
            MarginalizingBias dsolver_err{alpha*0, beta*0};
            dsolver_err.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}, {UUID{4}, 4}});
            dsolver_err.marginalize(J, x0, ids_a, ids_b);
            dsolver_err.update_indices({{UUID{ids_ka(0)}, 0}, {UUID{ids_ka(1)}, 1}, {UUID{ids_ka(2)}, 2}});
            Array<float> dx = dsolver_err.solve(J1, x01, residual);
            Array<float> x1 = x01 + dx;
            assert_allclose(x1, Array<float>{0.377255, 0.665708, -0.306312});

            // Array<size_t> ids_a{ArrayShape{0}};
            // Array<size_t> ids_b{ArrayShape{0}};
            // dsolver.marginalize(J1, x01, residual, ids_a, ids_b);

            // Array<float> residual = y - dot1d(J1.to_dense_array(), x1);

            // SparseArrayCcs<float> J2{J1};
            // Array<float> dx1 = dsolver.solve(J1, x1, residual);
            // Array<float> x2 = x1 + dx1;
            // assert_allclose(x2, Array<float>{0.377255, 0.665708, -0.306312});
        }
    }
}

void test_schur_solver() {
    Array<float> g = uniform_random_array<float>(ArrayShape{5, 5}, 1);
    Array<float> a = dot2d(g.vH(), g) + 2.f * identity_array<float>(g.shape(1));
    Array<float> b = uniform_random_array<float>(ArrayShape{5}, 2);
    Array<size_t> ids_a{0, 2, 3};
    Array<size_t> ids_b{1, 4};
    SchurComplement sc{a, b, ids_a, ids_b};

    assert_allclose(sc.solve(0, 0), lstsq_chol_1d(a, b));
}

void test_fill_in() {
    UUIDGen<CameraVariable, FeaturePointVariable> uuid_gen;

    TestScene1d tc(uuid_gen);
    gen_scene1(tc);
    Scene1dMatrix m{tc};
    // std::cerr << (m.jacobian().to_dense_array() != 0.f) << std::endl;
    {
        MarginalizingBias bsolver{0, 0};
        bsolver.update_indices(m.predictor_uuids());
        Array<float> x0 = uniform_random_array<float>(ArrayShape{m.jacobian().shape(1)}, 1);
        Array<size_t> ids_k{
            m.column_id_feature_point(FeaturePointVariable{1}),
            m.column_id_feature_point(FeaturePointVariable{2}),
            m.column_id_feature_point(FeaturePointVariable{3}),
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(2)}),
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(3)})};
        Array<size_t> ids_a{
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(0)}),
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(1)})};
        Array<size_t> ids_b{m.column_id_feature_point(FeaturePointVariable{0})};
        Array<size_t> ids_ka = ids_k.appended(ids_a);
        Array<float> residual = m.rhs() - dot1d(m.jacobian().to_dense_array(), x0);
        Array<float> residual1 = m.rhs() - dot1d(m.jacobian().columns(ids_ka).to_dense_array(), x0.blocked(ids_ka));
        bsolver.marginalize(m.jacobian(), x0, ids_a, ids_b);
        bsolver.update_indices({
            {UUID{ids_ka(0)}, 0},
            {UUID{ids_ka(1)}, 1},
            {UUID{ids_ka(2)}, 2},
            {UUID{ids_ka(3)}, 3},
            {UUID{ids_ka(4)}, 4},
            {UUID{ids_ka(5)}, 5},
            {UUID{ids_ka(6)}, 6}});
        // std::cerr << bsolver.lhs_ka_ << std::endl;
        Array<float> dx = bsolver.solve(m.jacobian().columns(ids_ka), x0.blocked(ids_ka), residual);
        // std::cerr << dx << std::endl;
    }
    {
        MarginalizingBias bsolver{0, 0};
        bsolver.update_indices(m.predictor_uuids());
        Array<float> x0 = uniform_random_array<float>(ArrayShape{m.jacobian().shape(1)}, 1);
        Array<size_t> ids_k{
            m.column_id_feature_point(FeaturePointVariable{2}),
            m.column_id_feature_point(FeaturePointVariable{3}),
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(1)}),
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(2)}),
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(3)})};
        Array<size_t> ids_a{
            m.column_id_feature_point(FeaturePointVariable{0}),
            m.column_id_feature_point(FeaturePointVariable{1})};
        Array<size_t> ids_b{
            m.column_id_camera(CameraVariable{std::chrono::milliseconds(0)})};
        Array<size_t> ids_ka = ids_k.appended(ids_a);
        Array<float> residual = m.rhs() - dot1d(m.jacobian().to_dense_array(), x0);
        Array<float> residual1 = m.rhs() - dot1d(m.jacobian().columns(ids_ka).to_dense_array(), x0.blocked(ids_ka));
        bsolver.marginalize(m.jacobian(), x0, ids_a, ids_b);
        bsolver.update_indices({
            {UUID{ids_ka(0)}, 0},
            {UUID{ids_ka(1)}, 1},
            {UUID{ids_ka(2)}, 2},
            {UUID{ids_ka(3)}, 3},
            {UUID{ids_ka(4)}, 4},
            {UUID{ids_ka(5)}, 5},
            {UUID{ids_ka(6)}, 6}});
        // std::cerr << bsolver.lhs_ka_ << std::endl;
        Array<float> dx = bsolver.solve(m.jacobian().columns(ids_ka), x0.blocked(ids_ka), residual);
        // std::cerr << dx << std::endl;
    }
}

int main(int argc, char** argv) {
    test_schur_complement2();
    test_schur_complement3();
    test_schur_solver();
    test_fill_in();
    return 0;
}
