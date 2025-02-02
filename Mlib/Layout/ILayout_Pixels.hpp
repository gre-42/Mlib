#pragma once

namespace Mlib {

struct LayoutConstraintParameters;

enum class PixelsRoundMode;

class ILayoutPixels {
public:
    virtual ~ILayoutPixels() = default;
    virtual float to_pixels(
        const LayoutConstraintParameters& params,
        PixelsRoundMode round_mode) const = 0;
};

}
