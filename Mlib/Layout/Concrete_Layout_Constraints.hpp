#pragma once
#include <Mlib/Layout/ILayout_Scalar.hpp>

namespace Mlib {

enum class ScreenUnits;

class MaximumConstraint: public ILayoutScalar {
public:
    virtual float to_pixels(float dpi, int screen_npixels) const override;
};

class ConstantConstraint: public ILayoutScalar {
public:
    ConstantConstraint(
        float f,
        ScreenUnits screen_units);
    virtual float to_pixels(float dpi, int screen_npixels) const override;
private:
    float f_;
    ScreenUnits screen_units_;
};

class AdditiveConstraint: public ILayoutScalar {
public:
    AdditiveConstraint(
        float f,
        ScreenUnits screen_units,
        ILayoutScalar& a);
    virtual float to_pixels(float dpi, int screen_npixels) const override;
private:
    float f_;
    ScreenUnits screen_units_;
    ILayoutScalar& a_;
};

class FractionalConstraint: public ILayoutScalar {
public:
    FractionalConstraint(
        float f,
        ILayoutScalar& a,
        ILayoutScalar& b);
    virtual float to_pixels(float dpi, int screen_npixels) const override;
private:
    float f_;
    ILayoutScalar& a_;
    ILayoutScalar& b_;
};

}
