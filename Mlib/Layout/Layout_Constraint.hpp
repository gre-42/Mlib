#pragma once
#include <map>
#include <string>

namespace Mlib {

class LayoutConstraint {
public:
    virtual ~LayoutConstraint() = default;
    virtual float evaluate(float dpi, int screen_npixels) const = 0;
};

}
