#pragma once
#include <string>

namespace Mlib {

enum class RenderingDynamics {
    STATIC,
    MOVING
};

RenderingDynamics rendering_dynamics_from_string(const std::string& s);

}
