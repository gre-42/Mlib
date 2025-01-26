#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/Concrete_Layout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Data_Display/Circular_Data_Display.hpp>
#include <Mlib/Render/Data_Display/Pointer_Image_Logic.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Render/Text/Renderable_Text.hpp>
#include <Mlib/Render/Text/Text_Interpolation_Mode.hpp>
#include <Mlib/Render/Window.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Threads/Realtime_Threads.hpp>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv)
{
    try {
        if (argc != 3) {
            throw std::runtime_error("Usage: render_text filename.ttf texture.{png,jpg}");
        }

        reserve_realtime_threads(0);

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
            nullptr,    // monitor
            nullptr,    // share
            true,       // use_double_buffering
            1,          // swap_interval
            0};         // fullscreen_refresh_rate
        GlContextGuard gcg{ window };
        if (int version = gladLoadGL(glfwGetProcAddress); version == 0) {
            throw std::runtime_error("gladLoadGL failed");
        }

        // Resources
        // ---------
        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        TrailResources trail_resources;
        RenderingResources rendering_resources{
            "primary_rendering_resources",
            16 };
        RenderingContext primary_rendering_context{
            .scene_node_resources = scene_node_resources,
            .particle_resources = particle_resources,
            .trail_resources = trail_resources,
            .rendering_resources = rendering_resources,
            .z_order = 0 };
        RenderingContextGuard rcg{ primary_rendering_context };

        // OpenGL state
        // ------------
        ConstantConstraint large_font_height{100, ScreenUnits::PIXELS};
        ConstantConstraint small_font_height{20, ScreenUnits::PIXELS};
        ConstantConstraint line_distance{100.f, ScreenUnits::PIXELS};
        LayoutConstraintParameters ly{
            .dpi = 96.f,
            .min_pixel = 0.f,
            .end_pixel = 800.f};
        TextResource renderable_text{argv[1], {1.f, 0.5f, 0.5f}};

        TextResource circular_renderable_text{argv[1], {0.5f, 1.f, 0.5f}};
        std::vector<DisplayTick> ticks;
        for (int f = 0; f <= 100; f += 10) {
            ticks.push_back(DisplayTick{
                .value = (float)f,
                .text = std::to_string(f)});
        }
        PointerImageLogic pointer_image_logic{
            rendering_resources.get_texture(ColormapWithModifiers{
                .filename = VariableAndHash<std::string>{ argv[2] },
                .color_mode = ColorMode::RGBA,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS
            }.compute_hash())
        };
        CircularDataDisplay circular_data_display{
            circular_renderable_text,
            pointer_image_logic,
            30.f,           // minimum_value
            100.f,          // maximum_value
            90.f * degrees, // blank_angle
            ticks};

        float value = 0.f;
        // render loop
        // -----------
        while (!window.close_requested())
        {
            int width, height;

            GLFW_CHK(glfwGetFramebufferSize(&window.glfw_window(), &width, &height));
            CHK(glViewport(0, 0, width, height));

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            {
                PixelRegion ew{ 0.f, 400, 0.f, 500, RegionRoundMode::ENABLED };
                renderable_text.render(
                    large_font_height.to_pixels(ly, PixelsRoundMode::ROUND),
                    ew,
                    "12345\n54321\nThis is\nsample\ntext",
                    line_distance.to_pixels(ly, PixelsRoundMode::NONE),
                    TextInterpolationMode::NEAREST_NEIGHBOR);
            }
            {
                PixelRegion ew{ 10.f, 400, 120.f, 500.f, RegionRoundMode::ENABLED };
                circular_data_display.render(
                    value,              // value
                    20.f,               // font_height
                    TextInterpolationMode::NEAREST_NEIGHBOR,
                    ew,
                    100.f,              // tick_radius
                    { 7.f, 100.f });    // pointer_size
                value = std::fmod(value + 0.5f, 100.f);
            }

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            window.draw();
            GLFW_CHK(glfwPollEvents());
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
