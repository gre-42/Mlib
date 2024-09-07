#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Images/Linear_Interpolation.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Optimize/Find_Right_Boundary_Of_Maximum.hpp>
#include <Mlib/Math/Optimize/Newton_1D.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

namespace Mlib {

enum class MagicFormulaMode {
    STANDARD,
    NO_SLIP
};

template <class TData>
struct MagicFormula {
    TData B = 41;
    TData C = (TData)1.4;
    TData D = 1;
    TData E = (TData)-0.2;
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
class MagicFormulaArgmax {
public:
    MagicFormulaArgmax() {}
    explicit MagicFormulaArgmax(const MagicFormula<TData>& magic_formula)
        : mf{ magic_formula }
    {
        MagicFormula<double> mf2 = magic_formula.template casted<double>();
        double h = 1e-3;
        auto f = [&mf2](const double& x) { return (double)mf2(x); };
        auto df = [&f, &h](const double& x) { return (f(x + h) - f(x - h)) / (2 * h); };
        auto df2 = [&df, &h](const double& x) { return (df(x + h) - df(x - h)) / (2 * h); };
        double x0 = find_right_boundary_of_maximum<double>(f, 0, 1e-2);
        argmax = (TData)newton_1d(df, df2, x0);
        // return 1 / (B * std::sqrt(E - 1));
        TData xmin = 0.;
        TData xmax = TData(10) * argmax;
        ys = Array<TData>{ ArrayShape{ 1000 } };
        for (auto&& [i, x] : enumerate(Linspace(xmin, xmax, ys.length()))) {
            ys(i) = magic_formula(x);
        }
        li = { xmin, xmax, ys };
    }
    TData operator () (const TData& x, MagicFormulaMode mode = MagicFormulaMode::STANDARD) const {
        switch (mode) {
        case MagicFormulaMode::STANDARD:
            // return mf(x);
            TData y;
            if (!li(std::abs(x), y)) {
                return ys(ys.length() - 1);
            }
            return sign(x) * y;
        case MagicFormulaMode::NO_SLIP:
            return std::abs(x) >= argmax ? sign(x) * mf.D : (*this)(x);
        default:
            THROW_OR_ABORT("Unknown magic formula mode");
        }
    }
    TData argmax;
private:
    MagicFormula<TData> mf;
    Array<TData> ys;
    LinearInterpolationDomain<TData> li;
};

/**
 * From: Brian Beckman, The Physics Of Racing Series, Part 25
 */
template <class TData>
struct CombinedMagicFormula {
    FixedArray<MagicFormulaArgmax<TData>, 2> f;
    FixedArray<TData, 2> operator () (const FixedArray<TData, 2>& x, MagicFormulaMode mode = MagicFormulaMode::STANDARD) const {
        FixedArray<TData, 2> s{
            x(0) / f(0).argmax,
            x(1) / f(1).argmax};
        TData p = std::sqrt(sum(squared(s)));
        if (p < 1e-12) {
            return FixedArray<TData, 2>{(TData)0, (TData)0};
        }
        return {
            s(0) / p * f(0)(p * f(0).argmax, mode),
            s(1) / p * f(1)(p * f(1).argmax, mode)};
    }
    const MagicFormulaArgmax<TData>& longitudinal() const {
        return f(0);
    }
    const MagicFormulaArgmax<TData>& lateral() const {
        return f(1);
    }
    MagicFormulaArgmax<TData>& longitudinal() {
        return f(0);
    }
    MagicFormulaArgmax<TData>& lateral() {
        return f(1);
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
