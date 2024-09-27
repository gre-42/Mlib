#pragma once
#include <Mlib/Layout/IWidget.hpp>

namespace Mlib {

class ILayoutPixels;
struct LayoutConstraintParameters;

class PixelRegion: public IPixelRegion {
public:
    PixelRegion(
        float left,
        float right,
        float bottom,
        float top,
        RegionRoundMode round_mode);
    PixelRegion(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly);
    static PixelRegion transformed(const IPixelRegion& ew, float dx, float dy);
    virtual float width() const override;
    virtual float height() const override;
    virtual float left() const override;
    virtual float right() const override;
    virtual float bottom() const override;
    virtual float top() const override;
private:
    float left_;
    float right_;
    float bottom_;
    float top_;
};

class Widget: public IWidget {
public:
    Widget(
        const ILayoutPixels& left,
        const ILayoutPixels& right,
        const ILayoutPixels& bottom,
        const ILayoutPixels& top);
    virtual std::unique_ptr<IPixelRegion> evaluate(
        const LayoutConstraintParameters& x,
        const LayoutConstraintParameters& y,
        YOrientation y_orientation,
        RegionRoundMode round_mode) const override;
private:
    const ILayoutPixels& left_;
    const ILayoutPixels& right_;
    const ILayoutPixels& bottom_;
    const ILayoutPixels& top_;
};

}
