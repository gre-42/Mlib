#pragma once

namespace Mlib {

class IPixelRegion;

struct LayoutConstraintParameters {
    static LayoutConstraintParameters child_x(
        const LayoutConstraintParameters& lx,
        const IPixelRegion& region);
    static LayoutConstraintParameters child_y(
        const LayoutConstraintParameters& ly,
        const IPixelRegion& region);
    float flength() const;
    int ilength() const;
    float dpi;
    float min_pixel;
    float end_pixel;
};

}
