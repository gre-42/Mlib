#pragma once
#include <cmath>

namespace Mlib {

/**
 * From: https://en.wikipedia.org/wiki/Hans_B._Pacejka
 * 
 * Modification: x in radians, not degrees.
 * => B = 0.714 * 180 / pi = 41
 */
template <class TData>
TData magic_formula(
    const TData& x,
    const TData& B = 41,
    const TData& C = 1.4,
    const TData& D = 1,
    const TData& E = -0.2)
{
    return D * std::sin(C * std::atan(B * x - E * (B * x - std::atan(B * x))));
}

}
