#include "Base_Cursor_Axis_Binding.hpp"
#include <cstdint>
#include <iomanip>
#include <sstream>

using namespace Mlib;

std::string BaseCursorAxisBinding::to_string() const {
    if (axis == SIZE_MAX) {
        return "";
    }
    std::stringstream sstr;
    sstr << "(axis: " << axis << " (" << std::setprecision(2) << sign_and_scale << "))";
    return sstr.str();
}
