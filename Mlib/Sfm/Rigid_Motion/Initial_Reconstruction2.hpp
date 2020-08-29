#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template<class TData>
struct RansacOptions;

namespace Sfm {

Array<float> initial_reconstruction(
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& ki,
    const Array<float>& y0,
    const Array<float>& y1,
    bool points_are_normalized = false,
    Array<float>* condition_number = nullptr);

Array<float> initial_reconstruction_x3(
    const Array<float>& R,
    const Array<float>& t,
    const Array<float>& ki,
    const Array<float>& y0,
    const Array<float>& y1,
    bool verbose = false);

void find_projection_matrices(
    const Array<float>& x,
    const Array<float>& y,
    const Array<float>* ki_precomputed,
    const Array<float>* kep_initial,
    Array<float>* ki_out = nullptr,
    Array<float>* ke_out = nullptr,
    Array<float>* kep_out = nullptr,
    Array<float>* x_out = nullptr,
    float alpha = 1e-5,
    float beta = 1e-5,
    float alpha2 = 0,
    float beta2 = 0,
    float min_redux = 1e-6,
    size_t niterations = 100,
    size_t nburnin = 5,
    size_t nmisses = 3,
    bool print_residual = false,
    bool nothrow = false,
    Array<float>* final_residual = nullptr,
    const float* max_residual = nullptr,
    bool differentiate_numerically = false);

void find_projection_matrices_ransac(
    const RansacOptions<float>& ro,
    const Array<float>& x,
    const Array<float>& y,
    const Array<float>* ki_precomputed,
    Array<float>* ki_out = nullptr,
    Array<float>* ke_out = nullptr,
    Array<float>* kep_out = nullptr,
    Array<float>* x_out = nullptr,
    float alpha = 1e-5,
    float beta = 1e-5,
    float alpha2 = 0,
    float beta2 = 0,
    float min_redux = 1e-6,
    size_t niterations = 100,
    size_t nburnin = 5,
    size_t nmisses = 3,
    bool print_residual = false,
    bool nothrow = false,
    Array<float>* final_residual = nullptr);

void find_projection_matrices_twopass(
    const Array<float>& x,
    const Array<float>& y,
    const Array<float>* ki_precomputed,
    Array<float>* ki_out = nullptr,
    Array<float>* ke_out = nullptr,
    Array<float>* kep_out = nullptr,
    Array<float>* x_out = nullptr,
    float alpha = 1e-5,
    float beta = 1e-5,
    float alpha2 = 0,
    float beta2 = 0,
    float min_redux = 1e-6,
    size_t niterations = 100,
    size_t nburnin = 5,
    size_t nmisses = 3,
    bool print_residual = false,
    bool nothrow = false,
    Array<float>* final_residual = nullptr,
    const float* max_residual = nullptr);

Array<float> find_epipole(
    const Array<float>& ki,
    const Array<float>& ke);

}}
