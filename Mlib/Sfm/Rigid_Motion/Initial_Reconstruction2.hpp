#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template<class TData>
struct RansacOptions;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Sfm {

Array<FixedArray<float, 3>> initial_reconstruction(
    const TransformationMatrix<float, float, 3>& ke,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    bool points_are_normalized = false,
    Array<float>* condition_number = nullptr);

Array<float> initial_reconstruction_x3(
    const TransformationMatrix<float, float, 3>& tm,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    bool verbose = false);

void find_projection_matrices(
    const Array<FixedArray<float, 3>>& x,
    const Array<FixedArray<float, 2>>& y,
    const TransformationMatrix<float, float, 2>* ki_precomputed,
    const Array<float>* kep_initial,
    TransformationMatrix<float, float, 2>* ki_out = nullptr,
    Array<TransformationMatrix<float, float, 3>>* ke_out = nullptr,
    Array<float>* kep_out = nullptr,
    Array<FixedArray<float, 3>>* x_out = nullptr,
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
    const Array<FixedArray<float, 3>>& x,
    const Array<FixedArray<float, 2>>& y,
    const TransformationMatrix<float, float, 2>* ki_precomputed,
    TransformationMatrix<float, float, 2>* ki_out = nullptr,
    Array<TransformationMatrix<float, float, 3>>* ke_out = nullptr,
    Array<float>* kep_out = nullptr,
    Array<FixedArray<float, 3>>* x_out = nullptr,
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
    const Array<FixedArray<float, 3>>& x,
    const Array<FixedArray<float, 2>>& y,
    const TransformationMatrix<float, float, 2>* ki_precomputed,
    TransformationMatrix<float, float, 2>* ki_out = nullptr,
    Array<TransformationMatrix<float, float, 3>>* ke_out = nullptr,
    Array<float>* kep_out = nullptr,
    Array<FixedArray<float, 3>>* x_out = nullptr,
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

FixedArray<float, 2> find_epipole(
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke);

}}
