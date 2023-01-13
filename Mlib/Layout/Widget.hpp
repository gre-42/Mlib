#pragma once
#include <Mlib/Layout/IWidget.hpp>

namespace Mlib {

class LayoutConstraint;

class EvaluatedWidget: public IEvaluatedWidget {
public:
    EvaluatedWidget(
        float left,
        float right,
        float bottom,
        float top);
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
        const LayoutConstraint& left,
        const LayoutConstraint& right,
        const LayoutConstraint& bottom,
        const LayoutConstraint& top);
    virtual std::unique_ptr<IEvaluatedWidget> evaluate(
        float xdpi,
        float ydpi,
        int xnpixels,
        int ynpixels,
        YOrientation y_orientation) const override;
private:
    const LayoutConstraint& left_;
    const LayoutConstraint& right_;
    const LayoutConstraint& bottom_;
    const LayoutConstraint& top_;
};

}
