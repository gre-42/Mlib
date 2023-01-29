#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Destruction_Guard.hpp>
#include <Mlib/Render/Window.hpp>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv)
{
    try {
        if (argc != 2) {
            throw std::runtime_error("Usage: render_text filename.ttf");
        }
        // glfw: initialize and configure
        // ------------------------------
        GLFW_CHK(glfwInit());
        DestructionGuard dg{[](){glfwTerminate();}};
        GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4));
        GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1));
        GLFW_CHK(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));

    #ifdef __APPLE__
        GLFW_CHK(glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE));
    #endif

        // glfw window creation
        // --------------------
        Window window{
            800,
            600,
            "LearnOpenGL",
            nullptr,
            nullptr,
            true,
            1};
        GlContextGuard gcg{ window };
        CHK(int version = gladLoadGL(glfwGetProcAddress));
        if (version == 0) {
            throw std::runtime_error("gladLoadGL failed");
        }

        // OpenGL state
        // ------------
        ConstantConstraint font_height{100, ScreenUnits::PIXELS};
        LayoutConstraintParameters ly{
            .dpi = 96.f,
            .min_pixel = 0.f,
            .max_pixel = 799.f};
        TextResource renderable_text{argv[1], font_height};

        // render loop
        // -----------
        while (!glfwWindowShouldClose(&window.glfw_window()))
        {
            int width, height;

            GLFW_CHK(glfwGetFramebufferSize(&window.glfw_window(), &width, &height));
            CHK(glViewport(0, 0, width, height));

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            PixelRegion ew{0.f, 399.f, 0.f, 99.f};
            ConstantConstraint line_distance{100, ScreenUnits::PIXELS};
            renderable_text.render(ly, ew, "This is sample text", line_distance);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            window.draw();
            GLFW_CHK(glfwPollEvents());
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
