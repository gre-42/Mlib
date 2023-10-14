#ifndef __ANDROID__
#include "Print_Monitor_Info.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Render/CHK.hpp>
#include <span>

using namespace Mlib;

void Mlib::print_monitor_info() {
	int monitor_count;
	GLFW_CHK(GLFWmonitor** monitors = glfwGetMonitors(&monitor_count));
	for (auto [i_monitor, monitor] : enumerate(std::span{monitors, monitors + monitor_count})) {
		linfo() << "Monitor " << i_monitor;
		int mode_count;
		GLFW_CHK(const GLFWvidmode* modes = glfwGetVideoModes(monitor, &mode_count));
		for (const auto& [i_mode, mode] : enumerate(std::span(modes, modes + mode_count))) {
			linfo() << "  " << mode.width << " x " << mode.height << " @ " << mode.refreshRate << " Hz";
		}
	}
}

#endif
