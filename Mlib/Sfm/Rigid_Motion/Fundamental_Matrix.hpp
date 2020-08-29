#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

Array<float> find_fundamental_matrix(
    const Array<float>& y0,
    const Array<float>& y1,
    bool method_inverse_iteration = true);

Array<float> fundamental_error(
    const Array<float>& F,
    const Array<float>& y0,
    const Array<float>& y1);

Array<float> fundamental_to_essential(const Array<float>& F, const Array<float>& intrinsic_matrix);

Array<float> find_epipole(const Array<float>& F);

void find_epiline(
    const Array<float>& F,
    const Array<float>& y,
    Array<float>& p,
    Array<float>& v);

Array<float> fundamental_from_camera(
    const Array<float>& intrinsic0,
    const Array<float>& intrinsic1,
    const Array<float>& R,
    const Array<float>& t);

}}
