#include "Smoothness_Target.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SmoothnessTarget Mlib::smoothness_target_from_string(const std::string& s) {
    if (s == "physics") {
        return SmoothnessTarget::PHYSICS;
    } else if (s == "render") {
        return SmoothnessTarget::RENDER;
    } else {
        THROW_OR_ABORT("Unknown smoothness target: \"" + s + '"');
    }
}
