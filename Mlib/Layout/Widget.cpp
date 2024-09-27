#include "Widget.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>

using namespace Mlib;

PixelRegion::PixelRegion(
    float left,
    float right,
    float bottom,
    float top,
    RegionRoundMode round_mode)
    : left_{ ::Mlib::round(left, round_mode == RegionRoundMode::ENABLED ? PixelsRoundMode::ROUND : PixelsRoundMode::NONE) }
    , right_{ ::Mlib::round(right, round_mode == RegionRoundMode::ENABLED ? PixelsRoundMode::ROUND : PixelsRoundMode::NONE) }
    , bottom_{ ::Mlib::round(bottom, round_mode == RegionRoundMode::ENABLED ? PixelsRoundMode::ROUND : PixelsRoundMode::NONE) }
    , top_{ ::Mlib::round(top, round_mode == RegionRoundMode::ENABLED ? PixelsRoundMode::ROUND : PixelsRoundMode::NONE) }
{}

PixelRegion::PixelRegion(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly)
    : PixelRegion{
        lx.min_pixel,
        lx.end_pixel,
        ly.min_pixel,
        ly.end_pixel,
        RegionRoundMode::DISABLED}
{}

PixelRegion PixelRegion::transformed(const IPixelRegion& ew, float dx, float dy) {
    return {
        ew.left() + dx,
        ew.right() + dx,
        ew.bottom() + dy,
        ew.top() + dy,
        RegionRoundMode::DISABLED};
}

float PixelRegion::width() const {
    return right_ - left_;
}

float PixelRegion::height() const {
    return top_ - bottom_;
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
    : left_{ left }
    , right_{ right }
    , bottom_{ bottom }
    , top_{ top }
{}

std::unique_ptr<IPixelRegion> Widget::evaluate(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    YOrientation y_orientation,
    RegionRoundMode round_mode) const
{
    auto rnd = (round_mode == RegionRoundMode::DISABLED)
        ? PixelsRoundMode::NONE
        : PixelsRoundMode::ROUND;

    if (y_orientation == YOrientation::AS_IS) {
        return std::make_unique<PixelRegion>(
            left_.to_pixels(lx, rnd),
            right_.to_pixels(lx, rnd),
            bottom_.to_pixels(ly, rnd),
            top_.to_pixels(ly, rnd),
            RegionRoundMode::DISABLED);
    }
    if (y_orientation == YOrientation::SWAPPED) {
        return std::make_unique<PixelRegion>(
            left_.to_pixels(lx, rnd),
            right_.to_pixels(lx, rnd),
            ::Mlib::round(ly.end_pixel - top_.to_pixels(ly, PixelsRoundMode::NONE), rnd),
            ::Mlib::round(ly.end_pixel - bottom_.to_pixels(ly, PixelsRoundMode::NONE), rnd),
            RegionRoundMode::DISABLED);
    }
    THROW_OR_ABORT("Unknown y-orientation");
}
