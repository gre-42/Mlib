#include "Normal_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

NormalType Mlib::normal_type_from_string(const std::string& str) {
    if (str == "face") {
        return NormalType::FACE;
    } else if (str == "vertex") {
        return NormalType::VERTEX;
    } else {
        THROW_OR_ABORT("Unknown normal type: \"" + str + '"');
    }
}
