#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Sfm/Disparity/Dense_Mapping.hpp>
#include <iostream>
#include <random>

using namespace Mlib;
using namespace Mlib::Sfm;

static float xsum(const Array<float>& v) {
    auto m = !isnan(v);
    auto n = count_nonzero(m);
    if (n == 0) {
        throw std::runtime_error("n == 0");
    }
    return (sum(v[m]) * v.nelements()) / n;
}

void test_numerical_differentiation() {
    Array<float> g = random_array4<float>(ArrayShape{6, 8}, 5);
    float theta = 5;
    float lambda = 0.1;
    float epsilon = 1.1;
    Array<float> dsi = 1.f + random_array4<float>(ArrayShape{3}.concatenated(g.shape()), 1);
    Array<float> d = 1.f + random_array4<float>(g.shape(), 2);
    Array<float> a = 1.f + random_array4<float>(g.shape(), 3);
    Array<float> q = random_array4<float>(
        Dm::regularizer == Dm::Regularizer::DIFFERENCE_OF_BOXES
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
            Dm::energy_dq(g, epsilon, d, q),
            numerical_differentiation(f, q.flattened()).reshaped(q.shape()),
            2e-3);
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
            Dm::energy_dd(g, theta, d, a, q)(3, 4),
            1e-3);
    }
    {
        float sigma = 0.1;
        auto f = [&](const Array<float>& qq) {
            return Array<float>{Dm::prox_sigma_fs(
                sigma,
                g,
                epsilon,
                d,
                qq.reshaped(q.shape()))};
        };
        assert_allclose(
            numerical_differentiation(f, q.flattened()).reshaped(q.shape()),
            Dm::prox_sigma_fs_dq(sigma, g, epsilon, d, q),
            1e-2);
    }
    {
        float tau = 0.1;
        auto f = [&](const Array<float>& dd) {
            return Array<float>{Dm::prox_tau_gs(
                tau,
                g,
                theta,
                dd.reshaped(d.shape()),
                a,
                q,
                true)}; // true = zero_sum
        };
        // std::cerr << numerical_differentiation(f, d.flattened()).reshaped(g.shape()) << std::endl;
        // std::cerr << Dm::prox_tau_gs_dd(tau, g, theta, d, a, q) << std::endl;
        // ignores boundary effects from transposition of the gradient
        assert_isclose(
            numerical_differentiation(f, d.flattened()).reshaped(g.shape())(3, 4),
            Dm::prox_tau_gs_dd(tau, g, theta, d, a, q)(3, 4),
            1e-2);
    }
}

void test_gauss_newton_step() {
    assert_isclose(Dm::gauss_newton_step(1, 5, [](float x){ return x * x; }), 0.f);
    assert_isclose(Dm::gauss_newton_step(1, 5, [](float x){ return (x - 0.5f) * (x - 0.5f); }), 0.5f);
    assert_isclose(Dm::gauss_newton_step(0, 5, [](float x){ return (x - 0.5f) * (x - 0.5f); }), 0.f);
    assert_isclose(Dm::gauss_newton_step(2, 5, [](float x){ return (x - 0.5f) * (x - 0.5f); }), 0.5f);
    assert_isclose(Dm::gauss_newton_step(2, 5, [](float x){ return (x + 0.5f) * (x + 0.5f); }), 0.f);  // clipping
    assert_isclose(Dm::gauss_newton_step(4, 5, [](float x){ return (x - 3.5f) * (x - 3.5f); }), 3.5f);
    assert_isclose(Dm::gauss_newton_step(5, 5, [](float x){ return (x - 3.5f) * (x - 3.5f); }), 4.f);
    assert_isclose(Dm::gauss_newton_step(4, 5, [](float x){ return (x - 6.5f) * (x - 6.5f); }), 5.f);  // clipping
    assert_isclose(Dm::gauss_newton_step(5, 5, [](float x){ return (x - 6.5f) * (x - 6.5f); }), 5.f);  // clipping
    assert_isclose(Dm::gauss_newton_step(5, 7, [](float x){ return (x - 6.5f) * (x - 6.5f); }), 6.5f);
}

void test_boundary_and_nan() {
    size_t nR = 5;
    size_t nH = 5;
    size_t nC = 4;
    Array<float> dsi = random_array4<float>(ArrayShape{nH, nR, nC}, 1);
    Array<float> g = random_array4<float>(ArrayShape{nR, nC}, 1);
    Dm::DtamParameters p;
    p.nsteps_ = 20;
    Array<float> a = Dm::dense_mapping(dsi, g, p);
    //std::cerr << a << std::endl;

    for(size_t dc = 0; dc < 2; ++dc) {
        Array<float> dsiN = full(ArrayShape{nH, nR, nC + 1}, NAN);
        Array<float> gN = full(ArrayShape{nR, nC + 1}, 0.45f);
        for(size_t h = 0; h < dsi.shape(0); ++h) {
            for(size_t r = 0; r < dsi.shape(1); ++r) {
                for(size_t c = 0; c < dsi.shape(2); ++c) {
                    dsiN(h, r, c + dc) = dsi(h, r, c);
                }
            }
        }
        for(size_t r = 0; r < g.shape(0); ++r) {
            for(size_t c = 0; c < g.shape(1); ++c) {
                gN(r, c + dc) = g(r, c);
            }
        }
        //std::cerr << dsi << std::endl;
        //std::cerr << g << std::endl;
        //std::cerr << dsiN << std::endl;
        //std::cerr << gN << std::endl;
        Array<float> aN = Dm::dense_mapping(dsiN, gN, p);
        //std::cerr << aN << std::endl;
        for(size_t r = 0; r < a.shape(0); ++r) {
            for(size_t c = 0; c < a.shape(1); ++c) {
                assert_isclose(aN(r, c + dc), a(r, c));
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
