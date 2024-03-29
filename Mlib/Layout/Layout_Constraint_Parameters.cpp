#include "Layout_Constraint_Parameters.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

float LayoutConstraintParameters::flength() const {
    return end_pixel - min_pixel;
}

int LayoutConstraintParameters::ilength() const {
    float f = flength();
    int i = (int)f;
    if ((float)i != f) {
        THROW_OR_ABORT("ilength() called on non-integer layout constraints");
    }
    return i;
}

LayoutConstraintParameters LayoutConstraintParameters::child_x(
    const LayoutConstraintParameters& lx,
    const IPixelRegion& region)
{
    return LayoutConstraintParameters{
        .dpi = lx.dpi,
        .min_pixel = region.left(),
        .end_pixel = region.right()};
}

LayoutConstraintParameters LayoutConstraintParameters::child_y(
    const LayoutConstraintParameters& ly,
    const IPixelRegion& region)
{
    return LayoutConstraintParameters{
        .dpi = ly.dpi,
        .min_pixel = region.bottom(),
        .end_pixel = region.top()};
}
