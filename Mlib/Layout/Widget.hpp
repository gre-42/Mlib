#pragma once
#include <Mlib/Layout/IWidget.hpp>

namespace Mlib {

class ILayoutPixels;

class EvaluatedWidget: public IEvaluatedWidget {
public:
    EvaluatedWidget(
        float left,
        float right,
        float bottom,
        float top);
    static EvaluatedWidget transformed(const IEvaluatedWidget& ew, float dx, float dy);
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
    virtual std::unique_ptr<IEvaluatedWidget> evaluate(
        float xdpi,
        float ydpi,
        int xnpixels,
        int ynpixels,
        YOrientation y_orientation) const override;
private:
    const ILayoutPixels& left_;
    const ILayoutPixels& right_;
    const ILayoutPixels& bottom_;
    const ILayoutPixels& top_;
};

}