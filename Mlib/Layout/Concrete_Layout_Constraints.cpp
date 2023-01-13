#include "Concrete_Layout_Constraints.hpp"
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

float MaximumConstraint::to_pixels(float dpi, int screen_npixels) const {
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

float ConstantConstraint::to_pixels(float dpi, int screen_npixels) const {
    return ::Mlib::to_pixels(screen_units_, f_, dpi, screen_npixels);
}

AdditiveConstraint::AdditiveConstraint(
    float f,
    ScreenUnits screen_units,
    ILayoutScalar& a)
: f_{f},
  screen_units_{screen_units},
  a_{a}
{}

float AdditiveConstraint::to_pixels(float dpi, int screen_npixels) const {
    return a_.to_pixels(dpi, screen_npixels) + ::Mlib::to_pixels(screen_units_, f_, dpi, screen_npixels);
}

FractionalConstraint::FractionalConstraint(
    float f,
    ILayoutScalar& a,
    ILayoutScalar& b)
: f_{f},
  a_{a},
  b_{b}
{}
    
float FractionalConstraint::to_pixels(float dpi, int screen_npixels) const {
    return
        (1 - f_) * a_.to_pixels(dpi, screen_npixels) +
        f_ * b_.to_pixels(dpi, screen_npixels);
}
