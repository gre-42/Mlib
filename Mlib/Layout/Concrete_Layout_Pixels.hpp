#pragma once
#include <Mlib/Layout/ILayout_Pixels.hpp>

namespace Mlib {

enum class PixelsRoundMode;
enum class ScreenUnits;

class MinimumConstraint: public ILayoutPixels {
public:
    virtual float to_pixels(
        const LayoutConstraintParameters& params,
        PixelsRoundMode round_mode) const override;
};

class EndConstraint: public ILayoutPixels {
public:
    virtual float to_pixels(
        const LayoutConstraintParameters& params,
        PixelsRoundMode round_mode) const override;
};

class ConstantConstraint: public ILayoutPixels {
public:
    ConstantConstraint(
        float f,
        ScreenUnits screen_units);
    virtual float to_pixels(
        const LayoutConstraintParameters& params,
        PixelsRoundMode round_mode) const override;
private:
    float f_;
    ScreenUnits screen_units_;
};

class LengthConstraint: public ILayoutPixels {
public:
    LengthConstraint(
        float f,
        ScreenUnits screen_units);
    virtual float to_pixels(
        const LayoutConstraintParameters& params,
        PixelsRoundMode round_mode) const override;
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
    virtual float to_pixels(
        const LayoutConstraintParameters& params,
        PixelsRoundMode round_mode) const override;
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
    virtual float to_pixels(
        const LayoutConstraintParameters& params,
        PixelsRoundMode round_mode) const override;
private:
    float f_;
    ILayoutPixels& a_;
    ILayoutPixels& b_;
};

}
