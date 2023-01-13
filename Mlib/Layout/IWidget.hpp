#pragma once
#include <memory>

namespace Mlib {

class IEvaluatedWidget {
public:
    virtual ~IEvaluatedWidget() = default;
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
    virtual std::unique_ptr<IEvaluatedWidget> evaluate(
        float xdpi,
        float ydpi,
        int xnpixels,
        int ynpixels,
        YOrientation y_orientation) const = 0;
};

}
