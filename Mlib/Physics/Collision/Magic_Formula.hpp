#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cmath>

namespace Mlib {

template <class TData>
struct MagicFormula {
    TData B = 41;
    TData C = 1.4;
    TData D = 1;
    TData E = -0.2;
    TData operator () (const TData& x) const {
        return D * std::sin(C * std::atan(B * x - E * (B * x - std::atan(B * x))));
    }
    TData argmax() const {
        return 1 / (B * std::sqrt(E - 1));
    }
};

/**
 * From: Brian Beckman, The Physics Of Racing Series, Part 25
 */
template <class TData>
struct CombinedMagicFormula {
    FixedArray<MagicFormula<TData>, 2> f;
    FixedArray<TData, 2> operator () (const FixedArray<TData, 2>& x) const {
        FixedArray<TData, 2> s{
            x(0) / f(0).argmax(),
            x(1) / f(1).argmax()};
        TData p = std::sqrt(sum(squared(s)));
        return {
            s(0) / p * f(0)(x(0) * p * f(0).argmax()),
            s(1) / p * f(1)(x(1) * p * f(1).argmax())};
    }
};

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
    return MagicFormula<TData>{.B = B, .C = C, .D = D, .E = E}(x);
}

}
