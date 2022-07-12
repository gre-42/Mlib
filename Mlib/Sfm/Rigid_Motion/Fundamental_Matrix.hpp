#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
template <class TData, size_t... tsize>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Sfm {

FixedArray<float, 3, 3> find_fundamental_matrix(
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    bool method_inverse_iteration = false);

Array<float> fundamental_error(
    const FixedArray<float, 3, 3>& F,
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1);

FixedArray<float, 3, 3> fundamental_to_essential(const FixedArray<float, 3, 3>& F, const TransformationMatrix<float, float, 2>& intrinsic_matrix);

FixedArray<float, 2> find_epipole(const FixedArray<float, 3, 3>& F);

void find_epiline(
    const FixedArray<float, 3, 3>& F,
    const FixedArray<float, 2>& y,
    FixedArray<float, 2>& p,
    FixedArray<float, 2>& v);

FixedArray<float, 3, 3> fundamental_from_camera(
    const TransformationMatrix<float, float, 2>& intrinsic0,
    const TransformationMatrix<float, float, 2>& intrinsic1,
    const TransformationMatrix<float, float, 3>& pose);

}}
