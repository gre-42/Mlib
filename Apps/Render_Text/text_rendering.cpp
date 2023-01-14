#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: render_text filename.ttf" << std::endl;
        return 1;
    }
    // glfw: initialize and configure
    // ------------------------------
    GLFW_CHK(glfwInit());
    GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3));
    GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3));
    GLFW_CHK(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));

#ifdef __APPLE__
    GLFW_CHK(glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE));
#endif

    // glfw window creation
    // --------------------
    GLFW_CHK(GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr));
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        GLFW_CHK(glfwTerminate());
        return -1;
    }
    GLFW_CHK(glfwMakeContextCurrent(window));
    CHK(int version = gladLoadGL(glfwGetProcAddress));
    if (version == 0) {
        throw std::runtime_error("gladLoadGL failed");
    }

    // OpenGL state
    // ------------
    ConstantConstraint font_height{100, ScreenUnits::PIXELS};
    TextResource renderable_text{argv[1], font_height};

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        int width, height;

        GLFW_CHK(glfwGetFramebufferSize(window, &width, &height));
        CHK(glViewport(0, 0, width, height));

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        EvaluatedWidget ew{0.f, 0.f, 100.f, 100.f};
        ConstantConstraint line_distance{100, ScreenUnits::PIXELS};
        renderable_text.render(100, 96.f, ew, "This is sample text", line_distance);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        GLFW_CHK(glfwSwapBuffers(window));
        GLFW_CHK(glfwPollEvents());
    }

    GLFW_CHK(glfwTerminate());
}
