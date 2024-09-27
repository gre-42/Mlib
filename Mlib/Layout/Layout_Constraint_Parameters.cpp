#include "Layout_Constraint_Parameters.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

float LayoutConstraintParameters::flength() const {
    return end_pixel - min_pixel;
}

int LayoutConstraintParameters::ilength() const {
    return float_to_integral<int>(end_pixel) - float_to_integral<int>(min_pixel);
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
