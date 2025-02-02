#include "Concrete_Layout_Pixels.hpp"
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

float MinimumConstraint::to_pixels(
    const LayoutConstraintParameters& params,
    PixelsRoundMode round_mode) const
{
    if (std::isnan(params.min_pixel)) {
        THROW_OR_ABORT("Minimum pixel requested, but min_pixel is NAN");
    }
    return ::Mlib::round(params.min_pixel, round_mode);
}

float EndConstraint::to_pixels(
    const LayoutConstraintParameters& params,
    PixelsRoundMode round_mode) const
{
    if (std::isnan(params.end_pixel)) {
        THROW_OR_ABORT("End pixel requested, but max_pixel is NAN");
    }
    return ::Mlib::round(params.end_pixel, round_mode);
}

ConstantConstraint::ConstantConstraint(
    float f,
    ScreenUnits screen_units)
    : f_{ f }
    , screen_units_{ screen_units }
{}

float ConstantConstraint::to_pixels(
    const LayoutConstraintParameters& params,
    PixelsRoundMode round_mode) const
{
    return ::Mlib::round(::Mlib::to_pixels(screen_units_, f_, params.dpi), round_mode);
}

AdditiveConstraint::AdditiveConstraint(
    float f,
    ScreenUnits screen_units,
    ILayoutPixels& a)
    : f_{ f }
    , screen_units_{ screen_units }
    , a_{ a }
{}

float AdditiveConstraint::to_pixels(
    const LayoutConstraintParameters& params,
    PixelsRoundMode round_mode) const
{
    return ::Mlib::round(
        a_.to_pixels(params, PixelsRoundMode::NONE) + ::Mlib::to_pixels(screen_units_, f_, params.dpi),
        round_mode);
}

FractionalConstraint::FractionalConstraint(
    float f,
    ILayoutPixels& a,
    ILayoutPixels& b)
    : f_{ f }
    , a_{ a }
    , b_{ b }
{}
    
float FractionalConstraint::to_pixels(
    const LayoutConstraintParameters& params,
    PixelsRoundMode round_mode) const
{
    return ::Mlib::round(
        (1 - f_) * a_.to_pixels(params, PixelsRoundMode::NONE) +
        f_ * b_.to_pixels(params, PixelsRoundMode::NONE),
        round_mode);
}
