
#include "Rendering_Dynamics.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

RenderingDynamics Mlib::rendering_dynamics_from_string(const std::string& s) {
    static const std::map<std::string, RenderingDynamics> m{
        {"static", RenderingDynamics::STATIC},
        {"moving", RenderingDynamics::MOVING}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown rendering dynamics: \"" + s + '"');
    }
    return it->second;
}
