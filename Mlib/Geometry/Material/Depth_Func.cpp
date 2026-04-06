
#include "Depth_Func.hpp"
#include <stdexcept>

using namespace Mlib;

DepthFunc Mlib::depth_func_from_string(const std::string& str) {
    if (str == "less") {
        return DepthFunc::LESS;
    } else if (str == "equal") {
        return DepthFunc::EQUAL;
    } else if (str == "less_equal") {
        return DepthFunc::LESS_EQUAL;
    }
    throw std::runtime_error("Unknown depth func: \"" + str + '"');
}
