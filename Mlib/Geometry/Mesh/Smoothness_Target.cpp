#include "Smoothness_Target.hpp"
#include <stdexcept>

using namespace Mlib;

SmoothnessTarget Mlib::smoothness_target_from_string(const std::string& s) {
    if (s == "physics") {
        return SmoothnessTarget::PHYSICS;
    } else if (s == "render") {
        return SmoothnessTarget::RENDER;
    } else {
        throw std::runtime_error("Unknown smoothness target: \"" + s + '"');
    }
}
