#include "Depth_Func.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DepthFunc Mlib::depth_func_from_string(const std::string& str) {
    if (str == "less") {
        return DepthFunc::LESS;
    } else if (str == "equal") {
        return DepthFunc::EQUAL;
    } else if (str == "less_equal") {
        return DepthFunc::LESS_EQUAL;
    }
    THROW_OR_ABORT("Unknown depth func: \"" + str + '"');
}
