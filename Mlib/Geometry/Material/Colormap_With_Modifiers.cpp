#include "Colormap_With_Modifiers.hpp"

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const ColormapWithModifiers& t) {
    ostr <<
        "filename: " << t.filename << '\n' <<
        "color_mode: " << (int)t.color_mode << '\n' <<
        "desaturate: " << (int)t.desaturate << '\n' <<
        "alpha: " << t.alpha << '\n' <<
        "histogram: " << t.histogram << '\n' <<
        "average: " << t.average << '\n' <<
        "multiply: " << t.multiply << '\n' <<
        "mean_color: " << t.mean_color << '\n' <<
        "lighten: " << t.lighten << '\n' <<
        "lighten_top: " << t.lighten_top << '\n' <<
        "lighten_bottom: " << t.lighten_bottom << '\n';
    return ostr;
}
