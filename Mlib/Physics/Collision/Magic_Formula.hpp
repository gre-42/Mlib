#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Optimize/Find_Maximum_Right_Boundary.hpp>
#include <Mlib/Math/Optimize/Newton_1D.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
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
    template <class TData2>
    MagicFormula<TData2> casted() const {
        return MagicFormula<TData2>{
            .B = B,
            .C = C,
            .D = D,
            .E = E};
    }
};

template <class TData>
struct MagicFormulaArgmax {
    explicit MagicFormulaArgmax(const MagicFormula<TData>& mf) {
        MagicFormula<double> mf2 = mf.template casted<double>();
        auto f = [&mf2](const TData& x){return mf2(x);};
        auto df = [&f](const TData& x){return (f(x + 1e-3) - f(x - 1e-3)) / 2e-3;};
        auto df2 = [&df](const TData& x){return (df(x + 1e-3) - df(x - 1e-3)) / 2e-3;};
        double x0 = find_maximum_right_boundary<double>(f, 0, 1e-2);
        argmax = newton_1d(df, df2, x0);
        // return 1 / (B * std::sqrt(E - 1));
    }
    TData operator () (const TData& x) const {
        return mf(x);
    }
    TData call_positive(const TData& x) const {
        return std::abs(x) >= argmax ? sign(x) * mf.D : (*this)(x);
    }
    MagicFormula<TData> mf;
    TData argmax;
};

/**
 * From: Brian Beckman, The Physics Of Racing Series, Part 25
 */
template <class TData>
struct CombinedMagicFormula {
    FixedArray<MagicFormulaArgmax<TData>, 2> f;
    FixedArray<TData, 2> operator () (const FixedArray<TData, 2>& x) const {
        FixedArray<TData, 2> s{
            x(0) / f(0).argmax,
            x(1) / f(1).argmax};
        TData p = std::sqrt(sum(squared(s)));
        return {
            s(0) / p * f(0)(x(0) * p * f(0).argmax),
            s(1) / p * f(1)(x(1) * p * f(1).argmax)};
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

template <class TData>
TData magic_formula_positive(
    const TData& x,
    const TData& B = 41,
    const TData& C = 1.4,
    const TData& D = 1,
    const TData& E = -0.2)
{
    return MagicFormula<TData>{.B = B, .C = C, .D = D, .E = E}.call_positive(x);
}

}
