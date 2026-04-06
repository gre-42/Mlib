
#include "Up_Axis.hpp"
#include <stdexcept>

using namespace Mlib;

UpAxis Mlib::up_axis_from_string(const std::string& s) {
    if (s == "y") {
        return UpAxis::Y;
    } else if (s == "z") {
        return UpAxis::Z;
    } else {
        throw std::runtime_error("Unknown up axis: \"" + s + '"');
    }
}
