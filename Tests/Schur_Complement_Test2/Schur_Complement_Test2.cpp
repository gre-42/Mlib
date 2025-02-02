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
        {1.f,       0.f,    0.f,    0.f},
        {1.1f,     -1.1f,   0.f,    0.f},
        {1.05f,    -1.05f,  0.f,    0.f},
        {1.2f,     -1.2f,   0.f,    0.f},
        {0.f,       1.f,   -2.f,    0.f},
        {0.f,       1.1f,  -2.1f,   0.f},
        {0.f,       1.05f, -2.f,    0.f},
        {0.f,       0.f,    3.f,   -2.f},
        {0.f,       0.f,    3.1f,  -2.1f},
        {0.f,       0.f,    0.05f, -2.f}};
    Array<float> x0{2.f, 3.f, 4.f, 5.f};
    Array<float> y{
        5.f, -0.1f, 0.3f, 0.2f, -0.15f, 0.25f, 0.2f, 0.1f, -0.15f, 0.05f};
    SparseArrayCcs<float> J{A};
    Array<float> residual = y - dot1d(A, x0);
    Array<float> dx = lstsq_chol_1d(J, residual).value();
    assert_allclose(x0 + dx, Array<float>{5.00308f, 4.88531f, 2.4728f, 3.70942f}, (float)1e-5);
    x0(0) = (x0 + dx)(0);  // This means that x(0) can be marginalized without error.
    residual.reassign(y - dot1d(A, x0));
    {
        Array<float> dx = lstsq_chol_1d(J, residual).value();
        assert_allclose(x0 + dx, Array<float>{5.00308f, 4.88531f, 2.4728f, 3.70942f}, (float)1e-5);
    }

    float alpha = (float)1e-2;
    float beta = (float)1e-2;
    Array<size_t> ids_k{ArrayShape{0}};
    Array<size_t> ids_a{1u, 2u, 3u};
    Array<size_t> ids_b{0};
    Array<size_t> ids_ka = ids_k.appended(ids_a);
    {
        Array<float> lhs_ka;
        Array<float> rhs_ka;
        //::Mlib::Debug::schur_complement_jacobian_system(J, residual, x0, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, ids_ka, alpha, beta);
        ::Mlib::Debug2::schur_complement_jacobian_system(J, residual, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, ids_ka, alpha, beta);
        //Array<float> dx = solve_symm_1d(lhs_ka.unblocked(ids_ka, ids_ka, ArrayShape{J.shape(1), J.shape(1)}, 0.f), rhs_ka.unblocked(ids_ka, J.shape(1), 0.f), alpha, beta);
        Array<float> dx = solve_symm_1d(lhs_ka, rhs_ka, alpha, beta).value();
        assert_isclose(sum(abs(lhs_ka - lhs_ka.T())), 0.f);
        //lerr() << dx + dx.unblocked(ids_ka, x0.length());
    }
    {
        Array<float> lhs_ka;
        Array<float> rhs_ka;
        marginalize_least_squares(J, residual, x0, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, alpha*0, beta*0);
        // lerr() << dot2d(J.vH(), J);
        // lerr() << lhs_ka.blocked(ids_ka, ids_ka);
        Array<float> dx = solve_symm_1d(lhs_ka.blocked(ids_ka, ids_ka), rhs_ka.blocked(ids_ka), alpha, beta).value();
        //lerr() << x0 + dx.unblocked(ids_ka, x0.length());
    }
    {
        Array<float> A = dot2d(J.vH(), J);
        Array<float> b = dot1d(J.vH(), residual);
        {
            Array<float> dx = solve_symm_1d(A, b).value();
            assert_allclose(x0 + dx, Array<float>{5.00308f, 4.88531f, 2.4728f, 3.70942f}, (float)1e-5);
        }
        {
            Array<float> dx = solve_symm_1d(A, b, alpha, beta).value();
            assert_allclose(x0 + dx, Array<float>{5.20492f, 5.14403f, 2.66678f, 4.01099f}, (float)1e-5);
        }
        {
            Array<float> lhs;
            Array<float> rhs;
            schur_complement_system(A, b, ids_a, ids_b, lhs, rhs, alpha, beta);
            Array<float> dx = solve_symm_1d(lhs, rhs, alpha, beta).value();
            assert_allclose(x0 + dx.unblocked(ids_a, x0.length()), Array<float>{NAN, 5.20331f, 2.69573f, 4.05394f}, (float)1e-5);
        }
        {
            Array<float> lhs;
            Array<float> rhs;
            schur_complement_system(A, b, ids_a, ids_b, lhs, rhs, 0, 0);
            Array<float> dx = solve_symm_1d(lhs, rhs, alpha, beta).value();
            assert_allclose(x0 + dx.unblocked(ids_a, x0.length()), Array<float>{NAN, 5.2151f, 2.70148f, 4.06247f}, (float)1e-5);
        }
    }
    {
        MarginalizingBias dsolver{alpha*0, beta*0};
        dsolver.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}});
        if (false) {
            Array<float> dx = dsolver.solve(J, x0, residual);
            assert_allclose(x0 + dx, Array<float>{5.00308f, 4.88531f, 2.4728f, 3.70942f}, (float)1e-5);
        }
        {
            Array<float> dx = solve_symm_1d(dot2d(J.vH(), J), dot1d(J.vH(), residual)).value();
            assert_allclose(x0 + dx, Array<float>{5.00308f, 4.88531f, 2.4728f, 3.70942f}, (float)1e-5);
        }
        // dsolver.marginalize(J, x0, ids_a, ids_b);
        // if (false) {
        //     lerr() << "rhs_ka_ " << dsolver.rhs_ka_;
        //     dsolver.update_indices({{UUID{1}, 0}, {UUID{2}, 1}, {UUID{3}, 2}});
        //     lerr() << "rhs_ka_ " << dsolver.rhs_ka_;
        //     Array<float> dx = dsolver.solve(J.columns(ids_ka), x0, residual);
        //     lerr() << "lhs_ka_";
        //     lerr() << dsolver.lhs_ka_;
        //     lerr() << "rhs_ka_";
        //     lerr() << dsolver.rhs_ka_;
        //     lerr() << x0 + dx.unblocked(ids_ka, x0.length());
        // }
    }
}

void test_schur_complement3() {
    float alpha = (float)1e-2;
    float beta = (float)1e-2;

    Array<float> G = random_array3<float>(ArrayShape{10, 5}, 1);
    SparseArrayCcs<float> J{G};
    Array<float> A = dot2d(G.vH(), G);
    Array<float> y = random_array3<float>(ArrayShape{10}, 2);
    Array<float> b = dot(G.T(), y);

    Array<float> x_exact = solve_symm_1d(A, b).value();
    Array<float> x0 = x_exact + Array<float>{0.1f, 0.f, 0.f, 0.f, 0.f};  // This means that all except x(0) can be marginalized without error.
    Array<float> residual = y - dot1d(G, x0);
    Array<float> b1 = dot(G.T(), residual);

    {
        Array<float> x = solve_symm_1d(A, b).value();
        assert_allclose(x, Array<float>{0.567378f, 0.293146f, 0.101899f, -0.318106f, 0.333852f}, (float)1e-5);
    }
    {
        Array<float> x = solve_symm_1d(A, b, alpha, beta).value();
        assert_allclose(x, Array<float>{0.546972f, 0.279436f, 0.102724f, -0.286586f, 0.330915f}, (float)1e-5);
    }
    Array<size_t> ids_k{ArrayShape{0}};
    Array<size_t> ids_a{0u, 1u, 4u};
    Array<size_t> ids_b{2u, 3u};
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
        Array<float> x = solve_symm_1d(lhs, rhs, 0.f, 0.f).value();
        assert_allclose(x.unblocked(ids_a, b.length()), Array<float>{0.567378f, 0.293146f, NAN, NAN, 0.333852f}, (float)1e-5);
    }
    {
        Array<float> lhs;
        Array<float> rhs;
        schur_complement_system(A, b1, ids_a, ids_b, lhs, rhs, 0, 0);
        Array<float> dx = solve_symm_1d(lhs, rhs, 0.f, 0.f).value();
        assert_allclose(x0 + dx.unblocked(ids_a, b.length()), Array<float>{0.567378f, 0.293146f, NAN, NAN, 0.333852f}, (float)1e-5);
    }
    {
        MarginalizingBias dsolver{alpha*0, beta*0};
        dsolver.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}, {UUID{4}, 4}});
        {
            Array<float> dx = dsolver.solve(J, x0, residual);
            Array<float> x = x0 + dx;
            assert_allclose(x, Array<float>{0.567378f, 0.293146f, 0.101903f, -0.31811f, 0.333852f}, (float)1e-5);
        }

        {
            // WIP
            MarginalizingBias dsolver_wip{alpha*0, beta*0};
            dsolver_wip.update_indices({{UUID{0}, 0}, {UUID{1}, 1}, {UUID{2}, 2}, {UUID{3}, 3}, {UUID{4}, 4}});
            dsolver_wip.marginalize_wip(J, x0, residual, ids_a, ids_b);
            dsolver_wip.update_indices({{UUID{ids_ka(0)}, 0}, {UUID{ids_ka(1)}, 1}, {UUID{ids_ka(2)}, 2}});
            Array<float> dx = dsolver_wip.solve(J1, x01, residual);
            Array<float> x1 = x01 + dx;
            assert_allclose(x1, Array<float>{0.567378f, 0.293146f, 0.333852f});
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
                assert_allclose(x1, Array<float>{0.567378f, 0.293146f, 0.333852f}, (float)1e-5);
            }
            {
                Array<float> dx = dsolver_exact.solve(J1, x01, residual);
                Array<float> x1 = x01 + dx;
                assert_allclose(x1, Array<float>{0.567378f, 0.293146f, 0.333852f}, (float)1e-5);
                assert_isclose(dsolver_exact.bias(x01), 1.41828549f, 1e-5);
            }

            // Exact2
            dsolver_exact.marginalize(J1, x_exact1, ids_a2, ids_b2);
            dsolver_exact.update_indices({{UUID{ids_ka2(0)}, 0}, {UUID{ids_ka2(1)}, 1}});
            {
                Array<float> dx2 = dsolver_exact.solve(J2, x02, residual);
                Array<float> x2 = x02 + dx2;
                assert_allclose(x2, Array<float>{0.567378f, 0.293146f}, (float)1e-5);
                assert_isclose(dsolver_exact.bias(x02), 0.783129930f, (float)1e-5);
            }

            // Exact3
            dsolver_exact.marginalize(J2, x_exact2, ids_a3, ids_b3);
            dsolver_exact.update_indices({{UUID{ids_ka3(0)}, 0}});
            {
                Array<float> dx3 = dsolver_exact.solve(J3, x03, residual);
                Array<float> x3 = x03 + dx3;
                assert_allclose(x3, Array<float>{0.567378f});
                assert_isclose(dsolver_exact.bias(x03), 0.299348384f, (float)1e-5);
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
            assert_allclose(x1, Array<float>{0.362772f, 0.199253f, 0.241683f});

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

    assert_allclose(sc.solve(0, 0), lstsq_chol_1d(a, b).value());
}

void test_fill_in() {
    UUIDGen<int, CameraVariable, FeaturePointVariable> uuid_gen;

    TestScene1d tc(uuid_gen);
    gen_scene1(tc);
    Scene1dMatrix m{tc};
    // lerr() << (m.jacobian().to_dense_array() != 0.f);
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
        // lerr() << bsolver.lhs_ka_;
        Array<float> dx = bsolver.solve(m.jacobian().columns(ids_ka), x0.blocked(ids_ka), residual);
        // lerr() << dx;
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
        // lerr() << bsolver.lhs_ka_;
        Array<float> dx = bsolver.solve(m.jacobian().columns(ids_ka), x0.blocked(ids_ka), residual);
        // lerr() << dx;
    }
}

int main(int argc, char** argv) {
    try {
        test_schur_complement2();
        test_schur_complement3();
        test_schur_solver();
        test_fill_in();
    } catch (const std::runtime_error& e) {
        lerr() << "ERROR: " << e.what();
        return 1;
    }
    return 0;
}
