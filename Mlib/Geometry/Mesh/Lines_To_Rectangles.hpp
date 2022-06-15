#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

/**
 * Create rectangle for line segment (b .. c), with given widths,
 * contained in crossings [aL; ...; aR] >-- (b -- c) --< [dL; ...; dR].
 */
bool lines_to_rectangles(
    FixedArray<double, 2>& p00,
    FixedArray<double, 2>& p01,
    FixedArray<double, 2>& p10,
    FixedArray<double, 2>& p11,
    const FixedArray<double, 2>& aL,
    const FixedArray<double, 2>& aR,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    const FixedArray<double, 2>& dL,
    const FixedArray<double, 2>& dR,
    double width_aLb,
    double width_aRb,
    double width_bcL,
    double width_bcR,
    double width_cdL,
    double width_cdR);

}
