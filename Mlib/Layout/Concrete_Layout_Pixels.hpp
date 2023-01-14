#pragma once
#include <Mlib/Layout/ILayout_Pixels.hpp>

namespace Mlib {

enum class ScreenUnits;

class MaximumConstraint: public ILayoutPixels {
public:
    virtual float to_pixels(float dpi, int screen_npixels) const override;
};

class ConstantConstraint: public ILayoutPixels {
public:
    ConstantConstraint(
        float f,
        ScreenUnits screen_units);
    virtual float to_pixels(float dpi, int screen_npixels) const override;
private:
    float f_;
    ScreenUnits screen_units_;
};

class AdditiveConstraint: public ILayoutPixels {
public:
    AdditiveConstraint(
        float f,
        ScreenUnits screen_units,
        ILayoutPixels& a);
    virtual float to_pixels(float dpi, int screen_npixels) const override;
private:
    float f_;
    ScreenUnits screen_units_;
    ILayoutPixels& a_;
};

class FractionalConstraint: public ILayoutPixels {
public:
    FractionalConstraint(
        float f,
        ILayoutPixels& a,
        ILayoutPixels& b);
    virtual float to_pixels(float dpi, int screen_npixels) const override;
private:
    float f_;
    ILayoutPixels& a_;
    ILayoutPixels& b_;
};

}
