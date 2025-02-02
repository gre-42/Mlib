#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>

namespace Mlib {

/**
 * From: https://stackoverflow.com/a/43454629/2292832
 */
inline FixedArray<float, 3> arbitrary_orthogonal(const FixedArray<float, 3>& vec)
{
    bool b0 = (vec(0) <  vec(1)) && (vec(0) <  vec(2));
    bool b1 = (vec(1) <= vec(0)) && (vec(1) <  vec(2));
    bool b2 = (vec(2) <= vec(0)) && (vec(2) <= vec(1));

    return cross(vec, FixedArray<float, 3>{float(b0), float(b1), float(b2)});
}

}
