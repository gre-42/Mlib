#include "Concrete_Layout_Constraints.hpp"
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

float MaximumConstraint::evaluate(float dpi, int screen_npixels) const {
    if (screen_npixels == 0) {
        THROW_OR_ABORT("screen_npixels is 0");
    }
    return (float)(screen_npixels - 1);
}

ConstantConstraint::ConstantConstraint(
    float f,
    ScreenUnits screen_units)
: f_{f},
  screen_units_{screen_units}
{}

float ConstantConstraint::evaluate(float dpi, int screen_npixels) const {
    return to_pixels(screen_units_, f_, dpi, screen_npixels);
}

AdditiveConstraint::AdditiveConstraint(
    float f,
    ScreenUnits screen_units,
    LayoutConstraint& a)
: f_{f},
  screen_units_{screen_units},
  a_{a}
{}

float AdditiveConstraint::evaluate(float dpi, int screen_npixels) const {
    return a_.evaluate(dpi, screen_npixels) + to_pixels(screen_units_, f_, dpi, screen_npixels);
}

FractionalConstraint::FractionalConstraint(
    float f,
    LayoutConstraint& a,
    LayoutConstraint& b)
: f_{f},
  a_{a},
  b_{b}
{}
    
float FractionalConstraint::evaluate(float dpi, int screen_npixels) const {
    return
        (1 - f_) * a_.evaluate(dpi, screen_npixels) +
        f_ * b_.evaluate(dpi, screen_npixels);
}
