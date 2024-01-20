#include "Concrete_Layout_Pixels.hpp"
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

float MinimumConstraint::to_pixels(const LayoutConstraintParameters& params) const {
    if (std::isnan(params.min_pixel)) {
        THROW_OR_ABORT("Minimum pixel requested, but min_pixel is NAN");
    }
    return params.min_pixel;
}

float MaximumConstraint::to_pixels(const LayoutConstraintParameters& params) const {
    if (std::isnan(params.max_pixel)) {
        THROW_OR_ABORT("Maximum pixel requested, but max_pixel is NAN");
    }
    return params.max_pixel;
}

ConstantConstraint::ConstantConstraint(
    float f,
    ScreenUnits screen_units)
    : f_{ f }
    , screen_units_{ screen_units }
{}

float ConstantConstraint::to_pixels(const LayoutConstraintParameters& params) const {
    return ::Mlib::to_pixels(screen_units_, f_, params.dpi);
}

AdditiveConstraint::AdditiveConstraint(
    float f,
    ScreenUnits screen_units,
    ILayoutPixels& a)
    : f_{ f }
    , screen_units_{ screen_units }
    , a_{ a }
{}

float AdditiveConstraint::to_pixels(const LayoutConstraintParameters& params) const {
    return a_.to_pixels(params) + ::Mlib::to_pixels(screen_units_, f_, params.dpi);
}

FractionalConstraint::FractionalConstraint(
    float f,
    ILayoutPixels& a,
    ILayoutPixels& b)
    : f_{ f }
    , a_{ a }
    , b_{ b }
{}
    
float FractionalConstraint::to_pixels(const LayoutConstraintParameters& params) const {
    return
        (1 - f_) * a_.to_pixels(params) +
        f_ * b_.to_pixels(params);
}
