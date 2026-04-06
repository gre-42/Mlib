
#ifndef __ANDROID__
#include "Print_Monitor_Info.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Iterator/Span.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <span>

using namespace Mlib;

void Mlib::print_monitor_info() {
    int monitor_count;
    GLFWmonitor** monitors = GLFW_CHK_X(glfwGetMonitors(&monitor_count));
    for (auto [i_monitor, monitor] : enumerate(Span{ monitors, integral_cast<size_t>(monitor_count) })) {
        linfo() << "Monitor " << i_monitor;
        int mode_count;
        const GLFWvidmode* modes = GLFW_CHK_X(glfwGetVideoModes(monitor, &mode_count));
        for (const auto& [i_mode, mode] : enumerate(Span(modes, integral_cast<size_t>(mode_count)))) {
            linfo() << "  " << mode.width << " x " << mode.height << " @ " << mode.refreshRate << " Hz";
        }
    }
}

#endif
