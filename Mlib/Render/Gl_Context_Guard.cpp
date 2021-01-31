#include "Gl_Context_Guard.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>

using namespace Mlib;

GlContextGuard::GlContextGuard(GLFWwindow* window) {
    GLFW_CHK(glfwMakeContextCurrent(window));
    execute_gc_render();

}

GlContextGuard::~GlContextGuard() {
    execute_gc_render();
    GLFW_CHK(glfwMakeContextCurrent(nullptr));
}
