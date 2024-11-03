#include "Rendering_Dynamics.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

RenderingDynamics Mlib::rendering_dynamics_from_string(const std::string& s) {
    static const std::map<std::string, RenderingDynamics> m{
        {"static", RenderingDynamics::STATIC},
        {"moving", RenderingDynamics::MOVING}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown rendering dynamics: \"" + s + '"');
    }
    return it->second;
}
