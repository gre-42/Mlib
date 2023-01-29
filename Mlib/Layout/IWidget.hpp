#pragma once
#include <memory>

namespace Mlib {

struct LayoutConstraintParameters;

class IPixelRegion {
public:
    virtual ~IPixelRegion() = default;
    virtual float width() const = 0;
    virtual float height() const = 0;
    virtual float left() const = 0;
    virtual float right() const = 0;
    virtual float bottom() const = 0;
    virtual float top() const = 0;
};

enum class YOrientation {
    AS_IS,
    SWAPPED
};

class IWidget {
public:
    virtual ~IWidget() = default;
    virtual std::unique_ptr<IPixelRegion> evaluate(
        const LayoutConstraintParameters& x,
        const LayoutConstraintParameters& y,
        YOrientation y_orientation) const = 0;
};

}
