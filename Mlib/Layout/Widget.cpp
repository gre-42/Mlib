#include "Widget.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>

using namespace Mlib;

PixelRegion::PixelRegion(
    float left,
    float right,
    float bottom,
    float top)
: left_{left},
  right_{right},
  bottom_{bottom},
  top_{top}
{}

PixelRegion::PixelRegion(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly)
: PixelRegion{lx.min_pixel, lx.max_pixel, ly.min_pixel, ly.max_pixel}
{}

PixelRegion PixelRegion::transformed(const IPixelRegion& ew, float dx, float dy) {
    return PixelRegion(
        ew.left() + dx,
        ew.right() + dx,
        ew.bottom() + dy,
        ew.top() + dy);
}

float PixelRegion::width() const {
    return right_ - left_ + 1;
}

float PixelRegion::height() const {
    return top_ - bottom_ + 1;
}

float PixelRegion::left() const {
    return left_;
}

float PixelRegion::right() const {
    return right_;
}

float PixelRegion::bottom() const {
    return bottom_;
}

float PixelRegion::top() const {
    return top_;
}

Widget::Widget(
    const ILayoutPixels& left,
    const ILayoutPixels& right,
    const ILayoutPixels& bottom,
    const ILayoutPixels& top)
: left_{left},
  right_{right},
  bottom_{bottom},
  top_{top}
{}

std::unique_ptr<IPixelRegion> Widget::evaluate(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    YOrientation y_orientation) const
{
    if (y_orientation == YOrientation::AS_IS) {
        return std::make_unique<PixelRegion>(
            left_.to_pixels(lx),
            right_.to_pixels(lx),
            bottom_.to_pixels(ly),
            top_.to_pixels(ly));
    }
    if (y_orientation == YOrientation::SWAPPED) {
        return std::make_unique<PixelRegion>(
            left_.to_pixels(lx),
            right_.to_pixels(lx),
            ly.max_pixel - top_.to_pixels(ly),
            ly.max_pixel - bottom_.to_pixels(ly));
    }
    THROW_OR_ABORT("Unknown y-orientation");
}
