#include <Mlib/Math/Debug/Schur_Complement_Debug.hpp>
#include <Mlib/Math/Debug/Schur_Complement_Debug2.hpp>
#include <Mlib/Math/Schur_Complement.hpp>
#include <Mlib/Sfm/Marginalization/Marginalizing_Bias.hpp>
#include <Mlib/Sfm/Marginalization/Regrid_Array.hpp>
#include <Mlib/Sfm/Marginalization/Synthetic_Scene.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <Mlib/Stats/Mean.hpp>
using namespace Mlib;

/*

void schur_complement_test0() {
    XUUIDGen uuid_gen;

    TestScene1d tc(uuid_gen);
    gen_scene0(tc);
    Scene1dMatrix m{tc};

    TestScene1d tc1(uuid_gen);
    gen_scene0(tc1);
    tc1.delete_feature_point(0);
    Scene1dMatrix m1{tc1};

    TestScene1d tc2(uuid_gen);
    gen_scene0(tc2);
    tc2.delete_feature_point(0);
    tc2.delete_feature_point(1);
    Scene1dMatrix m2{tc2};

    const auto c2i = [&](size_t i){ return m.column_id_camera(CameraVariable(std::chrono::milliseconds(i))); };
    const auto p2i = [&](size_t i){ return m.column_id_feature_point(FeaturePointVariable(i)); };
    const auto i1_c2i = [&](size_t i){ return m1.column_id_camera(CameraVariable(std::chrono::milliseconds(i))); };
    const auto i1_p2i = [&](size_t i){ return m1.column_id_feature_point(FeaturePointVariable(i)); };

    Array<size_t> ids_k{p2i(1)};
    Array<size_t> ids_a{c2i(0), c2i(1)};
    Array<size_t> ids_b{p2i(0)};
    Array<size_t> ids_ka = ids_k.appended(ids_a);

    Array<float> rhs_orig{m.rhs()};
    SparseArrayCcs<float> J{m.jacobian()};
    Array<float> x0 = random_array4<float>(ArrayShape{J.shape(1)}, 1);
    Array<float> residual = rhs_orig - dot1d(J.to_dense_array(), x0);
    if (true) {
        std::cerr << "\n\nSolve only" << std::endl;
        Array<float> x = lstsq_chol_1d(J, rhs_orig, 1e-6);
        m.print_x(x);
    }
    {
        std::cerr << "\n\nMarginalize once" << std::endl;
        Array<float> lhs_ka;
        Array<float> rhs_ka;
        marginalize_least_squares(J, residual, x0, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, alpha, beta);
        Array<float> x_new = solve_symm_1d(lhs_ka, rhs_ka, alpha, beta);
        Array<float> x_new_unblocked = x_new.unblocked(ids_ka, x0.length(), NAN);
        m.print_x(x_new_unblocked);
    }
    {
        {
            MarginalizingBias ms{alpha, beta};
            std::cerr << "\n\nNo marginalize" << std::endl;
            ms.update_indices(m.predictor_uuids());
            ms.recompute_hessian(J, residual);

            Array<float> x = x0 + ms.solve();
            m.print_x(x);
            assert_allclose(x, Array<float>{0, 0, 0, 0});
        }

        MarginalizingBias ms{alpha, beta};
        {
            std::cerr << "\n\nMarginalize" << std::endl;
            ms.update_indices(m.predictor_uuids());
            ms.recompute_hessian(J, residual);

            ms.marginalize(x0, ids_k, ids_a, ids_b);
            Array<float> x = x0 + ms.solve();
            m.print_x(x);
            assert_allclose(
                x - x(0),
                Array<float>{0.00000, 2.04591, NAN, 4.13904},
                1e-5);
        }

        {
            std::cerr << "\n\nMarginalize & recompute" << std::endl;
            SparseArrayCcs<float> J1 = m1.jacobian();
            Array<float> rhs_orig1{m1.rhs()};
            Array<float> x01 = random_array4<float>(ArrayShape{J1.shape(1)}, 2);
            Array<float> residual1 = rhs_orig1 - dot1d(J1.to_dense_array(), x01);
            //Array<float> residual1 = residual;
            //Array<float> x01 = lstsq_chol_1d(J1, residual1, alpha, beta);
            // std::cerr << "x01 " << x01 << std::endl;

            {
                std::cerr << "\n\nSolve new system" << std::endl;
                Array<float> x = lstsq_chol_1d(J1, rhs_orig1, 1e-6);
                m1.print_x(x);
                assert_allclose(
                    x - x(0),
                    Array<float>{0.00000, 2.04591, 4.13904},
                    1e-5);
            }
            std::cerr << std::endl;
            std::cerr << "x " << ms.x_ - mean(ms.x_) << std::endl;
            std::cerr << "rhs_ka_ " << ms.rhs_ka_ << std::endl;
            ms.update_indices(m1.predictor_uuids());
            ms.solve();
            std::cerr << std::endl;
            std::cerr << "x " << ms.x_ - mean(ms.x_)  << std::endl;
            {
                Array<float> x_new = ms.x_.unblocked(ms.ids_ka_, x01.length());
                m1.print_x(x_new);
                assert_allclose(
                    x_new - x_new(0),
                    Array<float>{0.00000, 2.0459, 4.13904},
                    1e-5);
            }

            std::cerr << "\n\nHessian" << std::endl;
            std::cerr << ms.lhs_ka_ << std::endl;
            std::cerr << ms.rhs_ka_ << std::endl;
            ms.recompute_hessian(J1);
            std::cerr << ms.lhs_ka_ << std::endl;
            std::cerr << ms.rhs_ka_ << std::endl;
            ms.solve();
            m1.print_x(ms.x_);
            assert_allclose(
                ms.x_ - ms.x_(0),
                Array<float>{0.00000, 2.0459, 4.13904},
                1e-5);

            std::cerr << "\n\nMarginalize x2" << std::endl;

            m1.print_uuids();
            Array<size_t> i1_ids_k{i1_c2i(0)};
            Array<size_t> i1_ids_a{i1_c2i(1)};
            Array<size_t> i1_ids_b{i1_p2i(1)};
            Array<size_t> i1_ids_ka = i1_ids_k.appended(i1_ids_a);

            SparseArrayCcs<float> J2 = m2.jacobian();
            Array<float> x02 = random_array4<float>(ArrayShape{J2.shape(1)}, 3);
            ms.marginalize(J1, residual1, x01, i1_ids_k, i1_ids_a, i1_ids_b, i1_ids_ka);
            ms.solve();
            {
                Array<float> x_new = ms.x_.unblocked(ms.ids_ka_, x01.length());
                m1.print_x(x_new);
                assert_allclose(
                    x_new - x_new(0),
                    Array<float>{0, 2.04591, NAN},
                    1e-5);
            }

            ms.update_indices(m2.predictor_uuids());
            ms.recompute_hessian(J2);
            ms.solve();
            m2.print_x(ms.x_);
            assert_allclose(
                ms.x_ - ms.x_(0),
                Array<float>{0, 2.04591},
                1e-5);
        }
    }
}

void schur_complement_test1() {
    UUIDGen<CameraVariable, FeaturePointVariable> uuid_gen;

    TestScene1d tc(uuid_gen);
    gen_scene1(tc);
    Scene1dMatrix m{tc};

    TestScene1d tc1(uuid_gen);
    gen_scene1(tc1);
    tc1.delete_feature_point(0);
    Scene1dMatrix m1{tc1};

    SparseArrayCcs<float> J{m.jacobian()};
    Array<float> rhs_orig{m.rhs()};
    if (true) {
        // x0 = 0 => residual = rhs
        // std::cerr << rhs_orig << std::endl;
        // std::cerr << J.to_dense_array() << std::endl;
        // std::cerr << dot2d(J.vH(), J) << std::endl;
        std::cerr << "\n\nSolve only" << std::endl;
        Array<float> x = lstsq_chol_1d(J, rhs_orig, 1e-6);
        m.print_x(x);
    }
    Array<float> x0 = random_array4<float>(ArrayShape{J.shape(1)}, 1);
    Array<float> residual = rhs_orig - dot1d(J.to_dense_array(), x0);
    const auto c2i = [&](size_t i){ return m.column_id_camera(CameraVariable(std::chrono::milliseconds(i))); };
    const auto p2i = [&](size_t i){ return m.column_id_feature_point(FeaturePointVariable(i)); };
    const auto c2i1 = [&](size_t i){ return m1.column_id_camera(CameraVariable(std::chrono::milliseconds(i))); };
    const auto p2i1 = [&](size_t i){ return m1.column_id_feature_point(FeaturePointVariable(i)); };
    // const auto c2u = [&](size_t i){ return tc.uuid_gen_.get(CameraVariable{std::chrono::milliseconds(i)}); };
    // const auto p2u = [&](size_t i){ return tc.uuid_gen_.get(FeaturePointVariable{i}); };

    if (false) {
        Array<size_t> i01_ids_k{c2i(3), p2i(2), p2i(3)};
        Array<size_t> i01_ids_a{c2i(0), c2i(1), c2i(2), c2i(4)};
        Array<size_t> i01_ids_b{p2i(0), p2i(1)};
        Array<size_t> i01_ids_ka = i01_ids_k.appended(i01_ids_a);

        {
            Array<float> lhs_ka;
            Array<float> rhs_ka;
            // Debug::schur_complement_jacobian_system(J, residual, x0, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, ids_ka, alpha, beta);
            // Debug2::schur_complement_jacobian_system(J, residual, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, ids_ka, alpha, beta);
            marginalize_least_squares(J, residual, x0, i01_ids_k, i01_ids_a, i01_ids_b, i01_ids_ka, lhs_ka, rhs_ka, alpha, beta);
            Array<float> x_new = solve_symm_1d(lhs_ka, rhs_ka, alpha, beta);
            // std::cerr << x_new << std::endl;
            Array<float> x_new_unblocked = x_new.unblocked(i01_ids_ka, x0.length(), NAN);
            // std::cerr << x_new_unblocked << std::endl;
            // tc.print_x(x_new_unblocked);
            m.print_x(x0 + x_new_unblocked);
            std::cerr << std::endl;
        }
        {
            MarginalizingSolver ms{alpha, beta};
            ms.update_indices(m.predictor_uuids());
            ms.recompute_hessian(J);
            ms.solve();
            // tc.print_x(x0 + ms.x_);
            ms.marginalize(J, residual, x0, i01_ids_k, i01_ids_a, i01_ids_b, i01_ids_ka);
            ms.solve();
            // std::cerr << ms.x_ << std::endl;
            Array<float> x_new_unblocked = ms.x_.unblocked(i01_ids_ka, x0.length(), NAN);
            m.print_x(x0 + x_new_unblocked);
            std::cerr << std::endl;
        }
    }
    {
        MarginalizingSolver ms{alpha, beta};
        Array<size_t> i1_ids_k{c2i(2), c2i(3), c2i(4), p2i(1), p2i(2), p2i(3)};
        Array<size_t> i1_ids_a{c2i(0), c2i(1)};
        Array<size_t> i1_ids_b{p2i(0)};
        Array<size_t> i1_ids_ka = i1_ids_k.appended(i1_ids_a);
        {
            m.print_uuids();
            std::cerr << std::endl;
            ms.update_indices(m.predictor_uuids());
            ms.recompute_hessian(J);

            // std::cerr << "i1_ids_b " << i1_ids_b << std::endl;
            // std::cerr << "i1_ids_ka " << i1_ids_ka << std::endl;

            ms.marginalize(J, residual, x0, i1_ids_k, i1_ids_a, i1_ids_b, i1_ids_ka);
            ms.solve();
            // std::cerr << ms.x_ << std::endl;
            std::cerr << "\n\nMarginalize" << std::endl;
            Array<float> x_new_unblocked = ms.x_.unblocked(i1_ids_ka, x0.length(), NAN);
            m.print_x(x0 + x_new_unblocked);
        }

        {
            std::cerr << "\n\nm1 print_uuids" << std::endl;
            m1.print_uuids();
            std::cerr << std::endl;
            SparseArrayCcs<float> J1 = m1.jacobian();
            Array<float> rhs_orig1{m1.rhs()};
            Array<float> x01 = random_array4<float>(ArrayShape{J1.shape(1)}, 1);
            Array<float> residual1 = rhs_orig1 - dot1d(J1.to_dense_array(), x01);

            {
                Array<float> x = lstsq_chol_1d(J1, rhs_orig1, 1e-6);
                m1.print_x(x);
            }
            std::cerr << "x " << ms.x_ - mean(ms.x_) << std::endl;
            // std::cerr << "lhs_ka_\n" << ms.lhs_ka_ << std::endl;
            std::cerr << "rhs_ka_ " << ms.rhs_ka_ << std::endl;
            ms.update_indices(m1.predictor_uuids());
            // ms.recompute_hessian(J1, residual1, true);
            ms.solve();
            std::cerr << "x " << ms.x_ - mean(ms.x_)  << std::endl;
            // std::cerr << "lhs_ka_\n" << ms.lhs_ka_ << std::endl;
            std::cerr << "rhs_ka_ " << ms.rhs_ka_ << std::endl;
            m1.print_x(x01 + ms.x_.unblocked(ms.ids_ka_, x01.length()));
            assert(false);
            Array<size_t> i2_ids_k{c2i1(3), p2i1(2), p2i1(3)};
            Array<size_t> i2_ids_a{c2i1(0), c2i1(1), c2i1(2), c2i1(4)};
            Array<size_t> i2_ids_b{p2i1(1)};
            Array<size_t> i2_ids_ka = i2_ids_k.appended(i2_ids_a);
            ms.marginalize(J1, residual, x0, i2_ids_k, i2_ids_a, i2_ids_b, i2_ids_ka);
            ms.solve();
            Array<float> x_new_unblocked = ms.x_.unblocked(i2_ids_ka, x01.length(), NAN);
            m1.print_x(x01 + x_new_unblocked);
        }
    }
}*/

int main(int argc, char** argv) {
    // schur_complement_test1();
    // schur_complement_test0();
    return 0;
}
