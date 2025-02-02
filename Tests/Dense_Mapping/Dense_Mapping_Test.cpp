#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping_Common.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Sfm;

static float xsum(const Array<float>& v) {
    auto m = !Mlib::isnan(v);
    auto n = count_nonzero(m);
    if (n == 0) {
        throw std::runtime_error("n == 0");
    }
    return (sum(v[m]) * v.nelements()) / n;
}

void test_numerical_differentiation() {
    Array<float> g = uniform_random_array<float>(ArrayShape{6, 8}, 5);
    float theta = 5.f;
    float lambda = 0.1f;
    float epsilon = 1.1f;
    Array<float> dsi = 1.f + uniform_random_array<float>(ArrayShape{3}.concatenated(g.shape()), 1);
    Array<float> d = 1.f + uniform_random_array<float>(g.shape(), 2);
    Array<float> a = 1.f + uniform_random_array<float>(g.shape(), 3);
    Array<float> q = uniform_random_array<float>(
        HuberRof::regularizer == HuberRof::Regularizer::DIFFERENCE_OF_BOXES
            ? g.shape()
            : ArrayShape{2}.concatenated(g.shape()),
        4) / float(g.nelements());
    {
        auto f = [&](const Array<float>& qq) {
            return Array<float>{xsum(Dm::energy(
                g,
                theta,
                lambda,
                epsilon,
                dsi,
                d,
                a,
                qq.reshaped(q.shape())))};
        };
        assert_allclose(
            HuberRof::energy_dq(g, epsilon, d, q),
            numerical_differentiation(f, q.flattened()).reshaped(q.shape()),
            float(2e-3));
    }
    {
        auto f = [&](const Array<float>& dd) {
            return Array<float>{xsum(Dm::energy(
                g,
                theta,
                lambda,
                epsilon,
                dsi,
                dd.reshaped(d.shape()),
                a,
                q))};
        };
        // ignores boundary effects from transposition of the gradient
        assert_isclose(
            numerical_differentiation(f, d.flattened()).reshaped(g.shape())(3, 4),
            HuberRof::energy_dd(g, theta, d, a, q)(3, 4),
            float(1e-3));
    }
    {
        float sigma = 0.1f;
        auto f = [&](const Array<float>& qq) {
            return Array<float>{HuberRof::prox_sigma_fs(
                sigma,
                g,
                epsilon,
                d,
                qq.reshaped(q.shape()))};
        };
        assert_allclose(
            numerical_differentiation(f, q.flattened()).reshaped(q.shape()),
            HuberRof::prox_sigma_fs_dq(sigma, g, epsilon, d, q),
            float(1e-2));
    }
    {
        float tau = 0.1;
        auto f = [&](const Array<float>& dd) {
            return Array<float>{HuberRof::prox_tau_gs(
                tau,
                g,
                theta,
                dd.reshaped(d.shape()),
                a,
                q,
                true)}; // true = zero_sum
        };
        // lerr() << numerical_differentiation(f, d.flattened()).reshaped(g.shape());
        // lerr() << Dm::prox_tau_gs_dd(tau, g, theta, d, a, q);
        // ignores boundary effects from transposition of the gradient
        assert_isclose(
            numerical_differentiation(f, d.flattened()).reshaped(g.shape())(3, 4),
            HuberRof::prox_tau_gs_dd(tau, g, theta, d, a, q)(3, 4),
            float(1e-2));
    }
}

void test_gauss_newton_step() {
    assert_isclose(gauss_newton_step(1, 5, [](float x){ return x * x; }), 0.f);
    assert_isclose(gauss_newton_step(1, 5, [](float x){ return (x - 0.5f) * (x - 0.5f); }), 0.5f);
    assert_isclose(gauss_newton_step(0, 5, [](float x){ return (x - 0.5f) * (x - 0.5f); }), 0.f);
    assert_isclose(gauss_newton_step(2, 5, [](float x){ return (x - 0.5f) * (x - 0.5f); }), 0.5f);
    assert_isclose(gauss_newton_step(2, 5, [](float x){ return (x + 0.5f) * (x + 0.5f); }), 0.f);  // clipping
    assert_isclose(gauss_newton_step(4, 5, [](float x){ return (x - 3.5f) * (x - 3.5f); }), 3.5f);
    assert_isclose(gauss_newton_step(5, 5, [](float x){ return (x - 3.5f) * (x - 3.5f); }), 4.f);
    assert_isclose(gauss_newton_step(4, 5, [](float x){ return (x - 6.5f) * (x - 6.5f); }), 5.f);  // clipping
    assert_isclose(gauss_newton_step(5, 5, [](float x){ return (x - 6.5f) * (x - 6.5f); }), 5.f);  // clipping
    assert_isclose(gauss_newton_step(5, 7, [](float x){ return (x - 6.5f) * (x - 6.5f); }), 6.5f);
}

void test_boundary_and_nan() {
    size_t nR = 5;
    size_t nH = 5;
    size_t nC = 4;
    Array<float> dsi = uniform_random_array<float>(ArrayShape{nH, nR, nC}, 1);
    Array<float> g = uniform_random_array<float>(ArrayShape{nR, nC}, 1);
    CostVolumeParameters c;
    Dm::DtamParameters p;
    p.nsteps_ = 20;
    Dm::DenseMapping dm{ g, c, p };
    dm.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
    dm.iterate_atmost(SIZE_MAX);
    //lerr() << a;

    for (size_t dc = 0; dc < 2; ++dc) {
        Array<float> dsiN = full(ArrayShape{nH, nR, nC + 1}, NAN);
        Array<float> gN = full(ArrayShape{nR, nC + 1}, 0.45f);
        for (size_t h = 0; h < dsi.shape(0); ++h) {
            for (size_t r = 0; r < dsi.shape(1); ++r) {
                for (size_t c = 0; c < dsi.shape(2); ++c) {
                    dsiN(h, r, c + dc) = dsi(h, r, c);
                }
            }
        }
        for (size_t r = 0; r < g.shape(0); ++r) {
            for (size_t c = 0; c < g.shape(1); ++c) {
                gN(r, c + dc) = g(r, c);
            }
        }
        //lerr() << dsi;
        //lerr() << g;
        //lerr() << dsiN;
        //lerr() << gN;
        Dm::DenseMapping dmN{ gN, c, p };
        dmN.notify_cost_volume_changed(InverseDepthCostVolume{ dsiN });
        dmN.iterate_atmost(SIZE_MAX);
        //lerr() << dmN.a_;
        for (size_t r = 0; r < dm.huber_rof_solver_.a_.shape(0); ++r) {
            for (size_t c = 0; c < dm.huber_rof_solver_.a_.shape(1); ++c) {
                assert_isclose(dmN.huber_rof_solver_.a_(r, c + dc), dm.huber_rof_solver_.a_(r, c));
            }
        }
    }
}

int main(int argc, char** argv) {
    test_numerical_differentiation();
    test_gauss_newton_step();
    test_boundary_and_nan();
    return 0;
}
