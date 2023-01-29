#pragma once

namespace Mlib {

struct LayoutConstraintParameters;

class ILayoutPixels {
public:
    virtual ~ILayoutPixels() = default;
    virtual float to_pixels(const LayoutConstraintParameters& params) const = 0;
};

}
