#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/CHK.hpp>
#include <chrono>
#include <thread>

using namespace Mlib;

int main(int argc, char** argv) {

    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Gamepad buttons", NULL, NULL);

    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Could not initialize window");
    }

    ButtonPress bp;
    while(true) {
        bp.update(window);
        bp.print();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    GLFW_CHK(glfwDestroyWindow(window));
    glfwTerminate();
    return 0;
}
