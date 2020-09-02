#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/CHK.hpp>
#include <chrono>
#include <thread>

using namespace Mlib;

int main(int argc, char** argv) {

    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }
    GLFW_CHK(glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE));

    GLFW_CHK(GLFWwindow* window = glfwCreateWindow(640, 480, "Gamepad buttons", NULL, NULL));

    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Could not initialize window");
    }

    ButtonStates bs;
    ButtonPress bp{bs};
    while(true) {
        bs.update_gamepad_state();
        bp.print();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    GLFW_CHK(glfwDestroyWindow(window));
    GLFW_CHK(glfwTerminate());
    return 0;
}
