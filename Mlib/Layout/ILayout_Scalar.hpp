#pragma once

namespace Mlib {

class ILayoutScalar {
public:
    virtual ~ILayoutScalar() = default;
    virtual float to_pixels(float dpi, int screen_npixels) const = 0;
};

}
