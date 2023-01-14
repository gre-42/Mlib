#pragma once

namespace Mlib {

class ILayoutPixels {
public:
    virtual ~ILayoutPixels() = default;
    virtual float to_pixels(float dpi, int screen_npixels) const = 0;
};

}
