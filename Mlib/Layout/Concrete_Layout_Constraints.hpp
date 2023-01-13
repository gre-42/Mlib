#pragma once
#include <Mlib/Layout/Layout_Constraint.hpp>

namespace Mlib {

enum class ScreenUnits;

class MaximumConstraint: public LayoutConstraint {
public:
    virtual float evaluate(float dpi, int screen_npixels) const override;
};

class ConstantConstraint: public LayoutConstraint {
public:
    ConstantConstraint(
        float f,
        ScreenUnits screen_units);
    virtual float evaluate(float dpi, int screen_npixels) const override;
private:
    float f_;
    ScreenUnits screen_units_;
};

class AdditiveConstraint: public LayoutConstraint {
public:
    AdditiveConstraint(
        float f,
        ScreenUnits screen_units,
        LayoutConstraint& a);
    virtual float evaluate(float dpi, int screen_npixels) const override;
private:
    float f_;
    ScreenUnits screen_units_;
    LayoutConstraint& a_;
};

class FractionalConstraint: public LayoutConstraint {
public:
    FractionalConstraint(
        float f,
        LayoutConstraint& a,
        LayoutConstraint& b);
    virtual float evaluate(float dpi, int screen_npixels) const override;
private:
    LayoutConstraint& a_;
    LayoutConstraint& b_;
    float f_;
};

}
