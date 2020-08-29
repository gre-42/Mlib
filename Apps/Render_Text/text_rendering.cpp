#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Fixed_Array.hpp>
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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    CHK(int version = gladLoadGL(glfwGetProcAddress));
    if (version == 0) {
        throw std::runtime_error("gladLoadGL failed");
    }

    // OpenGL state
    // ------------
    RenderableText renderable_text{argv[1]};

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

        renderable_text.render({100, 100}, "This is sample text", width, height);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
