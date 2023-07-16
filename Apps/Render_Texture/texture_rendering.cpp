#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Mesh/Load_Kn5.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv)
{
    try {
        if ((argc != 2) && (argc != 3)) {
            throw std::runtime_error("Usage: render_text <texture> [container.kn5]");
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

        // Resources
        // ---------
        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        auto rrg = RenderingContextGuard::root(
            scene_node_resources,
            particle_resources,
            "primary_rendering_resources",
            8,      // anisotropic_filtering_level
            0);     // z_order

        // OpenGL state
        // ------------
        if (argc == 3) {
            auto kn5 = load_kn5(argv[2]);
            RenderingContextStack::primary_rendering_resources()->insert_texture(
                argv[1],
                std::move(kn5.textures.at(argv[1])),
                TextureAlreadyExistsBehavior::RAISE);
        }
        FillWithTextureLogic ftl{argv[1], ResourceUpdateCycle::ONCE, ColorMode::RGB};

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

            ftl.render();

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
