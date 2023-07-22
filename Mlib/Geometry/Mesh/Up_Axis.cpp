#include "Up_Axis.hpp"

using namespace Mlib;

UpAxis Mlib::up_axis_from_string(const std::string& s) {
    if (s == "y") {
        return UpAxis::Y;
    } else if (s == "z") {
        return UpAxis::Z;
    } else {
        THROW_OR_ABORT("Unknown up axis: \"" + s + '"');
    }
}
