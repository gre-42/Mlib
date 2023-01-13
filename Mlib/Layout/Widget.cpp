#include "Widget.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/Layout_Constraint.hpp>

using namespace Mlib;

EvaluatedWidget::EvaluatedWidget(
    float left,
    float right,
    float bottom,
    float top)
: left_{left},
  right_{right},
  bottom_{bottom},
  top_{top}
{}

float EvaluatedWidget::width() const {
    return right_ - left_;
}

float EvaluatedWidget::height() const {
    return top_ - bottom_;
}

float EvaluatedWidget::left() const {
    return left_;
}

float EvaluatedWidget::right() const {
    return right_;
}

float EvaluatedWidget::bottom() const {
    return bottom_;
}

float EvaluatedWidget::top() const {
    return top_;
}

Widget::Widget(
    const LayoutConstraint& left,
    const LayoutConstraint& right,
    const LayoutConstraint& bottom,
    const LayoutConstraint& top)
: left_{left},
  right_{right},
  bottom_{bottom},
  top_{top}
{}

std::unique_ptr<IEvaluatedWidget> Widget::evaluate(
    float xdpi,
    float ydpi,
    int xnpixels,
    int ynpixels,
    YOrientation y_orientation) const
{
    if (y_orientation == YOrientation::AS_IS) {
        return std::make_unique<EvaluatedWidget>(
            left_.evaluate(xdpi, xnpixels),
            right_.evaluate(xdpi, xnpixels),
            bottom_.evaluate(ydpi, ynpixels),
            top_.evaluate(ydpi, ynpixels));
    }
    if (y_orientation == YOrientation::SWAPPED) {
        return std::make_unique<EvaluatedWidget>(
            left_.evaluate(xdpi, xnpixels),
            right_.evaluate(xdpi, xnpixels),
            (float)(ynpixels - 1) - top_.evaluate(ydpi, ynpixels),
            (float)(ynpixels - 1) - bottom_.evaluate(ydpi, ynpixels));
    }
    THROW_OR_ABORT("Unknown y-orientation");
}
