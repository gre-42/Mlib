#include "Gl_Context_Guard.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>

using namespace Mlib;

GlContextGuard::GlContextGuard(GLFWwindow* window) {
    GLFW_CHK(glfwMakeContextCurrent(window));
    execute_render_gc();

}

GlContextGuard::~GlContextGuard() {
    execute_render_gc();
    GLFW_CHK(glfwMakeContextCurrent(nullptr));
}
